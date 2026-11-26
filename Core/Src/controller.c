#include "controller.h"
#include <stdbool.h>
#include "motor.h"
#include "sensor.h"
#include "pid.h"

volatile PifWatch watch;
static volatile uint32_t last_ms;
static volatile bool fault;
static bool button_armed;
static uint16_t down_frames, up_frames, phase_frames;
static uint16_t base_now;
static uint16_t narrow_frames, wide_frames, reappear_frames;
static bool reappear_wide;
static float bend, last_turn;

static float magnitude(float x) {
    return x < 0 ? -x : x;
}

static float clamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}


static void drive(uint16_t left, uint16_t right) {
    Motor_SetDuty(left, right);
    watch.pwm_base = (left || right) ? base_now : 0U;
    watch.pwm_left = left;
    watch.pwm_right = right;
}

void Controller_Fault(void) {
    fault = true;
    drive(0, 0);
    watch.state = PIF_ADC_FAULT;
}

/* One event per press. A held button at power-up cannot start the car. */
static bool pressed(void) {
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) {
        down_frames = 0;
        if (up_frames < 15U) ++up_frames;
        if (up_frames == 15U) button_armed = true;
        return false;
    }
    up_frames = 0;
    if (down_frames < 15U) ++down_frames;
    if (!button_armed || down_frames < 15U) return false;
    button_armed = false;
    return true;
}

static void wheel_test(bool press) {
    static bool next_left = true;
    if (watch.state == PIF_IDLE && press) {
        watch.state = next_left ? PIF_LEFT_TEST : PIF_RIGHT_TEST;
        drive(next_left ? TEST_DUTY : 0U, next_left ? 0U : TEST_DUTY);
        next_left = !next_left;
        phase_frames = 0;
    } else if (watch.state != PIF_IDLE && (++phase_frames >= 250U || press)) {
        drive(0, 0);
        watch.state = PIF_IDLE;
    }
}

static void ramp_base(unsigned target) {
    if (base_now < target) ++base_now;
    else if (base_now > target) {
        unsigned step = base_now - target;
        base_now -= step > 8U ? 8U : step;
    }
}

static void mix(float turn) {
    float limit = clamp((float)(WHEEL_DUTY_CAP - base_now), 0, (float)base_now);
    last_turn = clamp(turn, -limit, limit);
    drive((uint16_t)(base_now + last_turn), (uint16_t)(base_now - last_turn));
}

static void track_narrow(float error) {
    float rate = Pid_Rate(error);
    /* Rising demand slows the car immediately. Slow release avoids a burst
       of throttle at the centre crossing between opposite turns. */
    float severity = clamp(magnitude(error)/22.0f + magnitude(rate)/850.0f, 0, 1);
    if (severity >= bend) bend = severity;
    else bend += 0.035f*(severity - bend);
    unsigned target = (unsigned)(0.5f + STRAIGHT_DUTY - bend*(STRAIGHT_DUTY - CORNER_DUTY));
    ramp_base(target);

    float limit = clamp((float)(WHEEL_DUTY_CAP - base_now), 0, (float)base_now);
    mix(Pid_Turn(error, limit));
}

/* Keep the time budget running across intermittent isolated good frames.
   Start and finish bars are only counted on consecutive unambiguous frames. */
static bool handle_gap(void) {
    if (watch.gap_frames == 0U) {
        if (watch.line_detected) return false;
        ++watch.gap_events;
    }
    if (watch.line_detected) {
        if (reappear_wide != (bool)watch.wide) reappear_frames = 0;
        reappear_wide = (bool)watch.wide;
        ++reappear_frames;
    } else reappear_frames = 0;
    if (reappear_frames >= RELOCK_FRAMES) {
        watch.gap_frames = 0;
        reappear_frames = 0;
        return false;
    }
    ++watch.gap_frames;
    narrow_frames = wide_frames = 0;
    bend = 1;
    Pid_Reset();
    if (watch.gap_frames >= GAP_STOP_FRAMES) {
        drive(0, 0);
        watch.state = PIF_LOST;
    } else {
        unsigned target = base_now < CORNER_DUTY ? base_now : CORNER_DUTY;
        ramp_base(target);
        mix(last_turn * 0.90f);
    }
    return true;
}

static void follow(float error) {
    if (handle_gap()) return;
    if (watch.wide) {
        ++wide_frames;
        narrow_frames = 0;
        if (wide_frames >= BAR_TIMEOUT_FRAMES ||
            (wide_frames >= BAR_ENTER_FRAMES && watch.state == PIF_RUNNING)) {
            watch.state = wide_frames >= BAR_TIMEOUT_FRAMES ? PIF_LOST : PIF_FINISHED;
            drive(0, 0);
            return;
        }
        if (wide_frames >= BAR_ENTER_FRAMES && watch.state == PIF_SEEK_START)
            watch.state = PIF_LEAVE_START;
        Pid_Reset();
        bend = 1;
        ramp_base(CORNER_DUTY);
        mix(0);
        return;
    }
    wide_frames = 0;
    if (narrow_frames < BAR_EXIT_FRAMES) ++narrow_frames;
    if (narrow_frames == BAR_EXIT_FRAMES && watch.state == PIF_LEAVE_START)
        watch.state = PIF_RUNNING;
    track_narrow(error);
}

static void tracking(bool press) {
    switch (watch.state) {
    case PIF_CAL_BAD:
    case PIF_WAIT_WHITE:
    case PIF_WAIT_BLACK:
        if (press) {
            Sensor_BeginCapture();
            watch.state = watch.state == PIF_WAIT_BLACK ? PIF_READ_BLACK : PIF_READ_WHITE;
        }
        return;
    case PIF_READ_WHITE: Sensor_Capture(true); return;
    case PIF_READ_BLACK: Sensor_Capture(false); return;
    default: break;
    }
    Sensor_Normalize();
    float e = Sensor_Position();
    switch (watch.state) {
    case PIF_READY:
        if (press && watch.line_detected && !watch.wide) {
            base_now = 0;
            bend = 1;
            last_turn = 0;
            watch.gap_events = watch.gap_frames = reappear_frames = 0;
            Pid_Reset();
            narrow_frames = wide_frames = 0;
            watch.state = PIF_SEEK_START;
        }
        break;
    case PIF_SEEK_START:
    case PIF_LEAVE_START:
    case PIF_RUNNING:
        if (press) {
            drive(0, 0);
            watch.state = PIF_READY;
        } else follow(e);
        break;
    case PIF_LOST:
    case PIF_FINISHED:
        if (press) watch.state = PIF_READY;
        break;
    default: break;
    }
}

void Controller_Frame(const uint16_t *src) {
    if (fault) return;
    last_ms = HAL_GetTick();
    for (unsigned i = 0; i < 9; ++i) watch.adc[i] = src[i];
    watch.pin_mV = (uint16_t)((uint32_t)src[8] * PACK_FULLSCALE_MV / 4095U);
    ++watch.frames;
    bool press = pressed();
    if (PIF_LESSON == PIF_WHEELS) wheel_test(press);
    if (PIF_LESSON == PIF_TRACK) tracking(press);
    /* LED active-low: steady in READY, fast blink on failure, slow otherwise. */
    bool led = (watch.state == PIF_READY) || ((watch.frames /
        ((watch.state == PIF_LOST || watch.state == PIF_CAL_BAD) ? 60U : 250U)) & 1U);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, led ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


void Pif_Init(ADC_HandleTypeDef *adc, TIM_HandleTypeDef *trigger, TIM_HandleTypeDef *pwm) {
    last_ms = HAL_GetTick();
    if (!Motor_Init(pwm)) {
        Controller_Fault();
        return;
    }
    watch.state = PIF_LESSON == PIF_TRACK ? PIF_WAIT_WHITE : PIF_IDLE;
    if (!Sensor_Start(adc, trigger)) Controller_Fault();
}

/* Poll in main; check with IRQs masked so a fresh frame cannot race the timeout. */
void Pif_Poll(void) {
    uint32_t key = __get_PRIMASK();
    __disable_irq();
    if (!fault && (uint32_t)(HAL_GetTick() - last_ms) > 20U) Controller_Fault();
    __set_PRIMASK(key);
    __WFI();
}
