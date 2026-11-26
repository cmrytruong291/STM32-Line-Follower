/* Host-side regression tests. The real firmware receives ADC frames through
   the same pair of HAL callbacks used by DMA on the STM32. */
#include "controller.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t now;
static uint16_t *dest, sample[9];
static int button = 1;
static unsigned bank;
static ADC_HandleTypeDef adc;
static TIM_HandleTypeDef pwm = {7199, {0, 0}}, tick;

int HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef *a) { return a != &adc; }
int HAL_TIM_PWM_Start(TIM_HandleTypeDef *p, unsigned c) { return p != &pwm || c > 1; }
int HAL_TIM_Base_Start(TIM_HandleTypeDef *t) { return t != &tick; }
int HAL_ADC_Start_DMA(ADC_HandleTypeDef *a, uint32_t *d, uint32_t n) {
    assert(a == &adc && n == 18);
    dest = (uint16_t *)(void *)d;
    return 0;
}
uint32_t HAL_GetTick(void) { return now; }
int HAL_GPIO_ReadPin(int port, int pin) {
    assert(port == GPIOB && pin == GPIO_PIN_10);
    return button;
}
void HAL_GPIO_WritePin(int port, int pins, int value) {
    (void)port; (void)pins; (void)value;
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *);

static void frames(unsigned count) {
    while (count--) {
        now += 2;
        memcpy(dest + bank*9, sample, sizeof sample);
        if (bank) HAL_ADC_ConvCpltCallback(&adc);
        else HAL_ADC_ConvHalfCpltCallback(&adc);
        bank ^= 1U;
        Pif_Poll();
        assert(watch.pwm_left <= WHEEL_DUTY_CAP && watch.pwm_right <= WHEEL_DUTY_CAP);
        assert(pwm.out[0] == 7200U*watch.pwm_left/1000U);
        assert(pwm.out[1] == 7200U*watch.pwm_right/1000U);
    }
}
static void press(void) {
    button = 0; frames(16);
    button = 1; frames(16);
}

static int white = 3000, black = 400;
static void fill(int value) {
    for (int i = 0; i < 8; ++i) sample[i] = (uint16_t)value;
}
static void line(int left, int right) {
    fill(white);
    sample[left] = sample[right] = (uint16_t)black;
}

static void calibrate_and_start(void) {
    fill(white); frames(16); press(); frames(64);
    press(); frames(64); /* Black still sees white. */
    assert(watch.state == PIF_CAL_BAD && watch.bad_mask == 255);
    press(); frames(64); assert(watch.state == PIF_WAIT_BLACK);
    fill(black); press(); frames(64); assert(watch.state == PIF_READY);
    line(3, 4); press(); frames(400);
    assert(watch.state == PIF_SEEK_START && watch.pwm_left == STRAIGHT_DUTY);
    fill(black); frames(6); assert(watch.state == PIF_LEAVE_START);
    line(3, 4); frames(400); assert(watch.state == PIF_RUNNING);
}

static void signals(void) {
    unsigned straight = watch.pwm_base;
    sample[0] = (uint16_t)black; frames(1); line(3,4); frames(2);
    assert(watch.error_mm == 0 && watch.gap_events == 0);
    line(5,6); frames(45);
    assert(watch.error_mm == 16 && watch.pwm_left > watch.pwm_right && watch.pwm_base < straight);
    line(1,2); frames(15); assert(watch.error_mm == -16 && watch.pwm_right > watch.pwm_left);
    fill(white); frames(5); assert(watch.state == PIF_RUNNING && watch.gap_frames > 0);
    line(2,3); frames(4); assert(watch.gap_frames == 0 && watch.error_mm == -8);
    fill((white + black)/2); frames(5);
    assert(!watch.line_detected && watch.contrast == 0 && watch.state == PIF_RUNNING);
    line(2,3); frames(4);
    line(0,7); frames(4); assert(!watch.line_detected && watch.gap_frames > 0);
    line(2,3); frames(4);
    /* There is no line. A single plausible spike every third frame cannot
       extend the grace interval. */
    unsigned elapsed;
    for (elapsed = 0; elapsed < 22 && watch.state == PIF_RUNNING; ++elapsed) {
        fill(white);
        if (elapsed % 3 == 2) line(1,2);
        frames(1);
    }
    assert(watch.state == PIF_LOST && elapsed <= 17 && !watch.pwm_left && !watch.pwm_right);
    line(3,4); frames(20); assert(watch.state == PIF_LOST);
    press(); assert(watch.state == PIF_READY);
    press(); assert(watch.state == PIF_SEEK_START);
    fill(black); frames(6); line(3,4); frames(8);
    fill(black); frames(2); line(3,4); frames(10); assert(watch.state == PIF_RUNNING);
    fill(black); frames(6); assert(watch.state == PIF_FINISHED && !watch.pwm_left && !watch.pwm_right);
}

static double wheel_speed(unsigned duty) {
    /* Unmeasured example model, mechanical dead zone expressed below
       as 60 per mille of input, 600 mm/s at maximum input. */
    return duty > 60 ? (duty - 60)*600.0/940.0 : 0.0;
}

static void waves(int noisy, double wavelength, double lookahead, double width, double lag) {
    /* Lab assumptions in millimetres: path y=25 sin(2*pi*x/wavelength), 20 mm line,
       lookahead and wheel spacing passed by test runner. */
    const double k = 2*3.141592653589793/wavelength, dt = 0.002, amplitude = 25;
    double x = 0, y = 0, heading = atan(amplitude*k), vl = 0, vr = 0;
    double square_error = 0, max_error = 0;
    uint32_t seed = 7;
    unsigned n = 0;
    while (x < 1200 && n < 25000 && watch.state == PIF_RUNNING) {
        double c = cos(heading), s = sin(heading);
        for (int i = 0; i < 8; ++i) {
            double offset = (i - 3.5)*8;
            double sx = x + lookahead*c - offset*s, sy = y + lookahead*s + offset*c;
            double distance = fabs(sy - amplitude*sin(k*sx)) / hypot(1,amplitude*k*cos(k*sx));
            double coverage = fmin(1, fmax(0,(13-distance)/6));
            seed = 1664525U*seed + 1013904223U;
            double disturbance = noisy ? 180*sin(3*x/100) + (int)(seed%141U)-70 : 0;
            double raw = white + (black-white)*coverage + disturbance;
            sample[i] = (uint16_t)fmax(0,fmin(4095,raw));
        }
        if (noisy && n%223 == 50) sample[(n/223)%8] = 4095;
        if (noisy && n%701 >= 300 && n%701 <= 303) fill(white);
        frames(1);
        vl += (wheel_speed(watch.pwm_left) - vl)*dt/lag;
        vr += (wheel_speed(watch.pwm_right)*0.97 - vr)*dt/lag;
        x += (vl+vr)*0.5*c*dt;
        y += (vl+vr)*0.5*s*dt;
        heading += (vl-vr)/width*dt;
        double e = fabs(y - amplitude*sin(k*x));
        square_error += e*e;
        if (e > max_error) max_error = e;
        ++n;
    }
    printf("waves period=%.0f mm, noisy=%d: x=%.1f mm, %u frames, axle lateral RMS=%.2f mm, peak=%.2f mm, gaps=%u, state=%d\n",
           wavelength,noisy,x,n,sqrt(square_error/n),max_error,(unsigned)watch.gap_events,(int)watch.state);
    assert(x >= 1200 && watch.state == PIF_RUNNING && max_error < 20);
    if (noisy) assert(watch.gap_events > 0);
}

int main(int argc, char **argv) {
    const char *scenario = argc > 1 ? argv[1] : "signals";
    double lookahead = argc > 2 ? atof(argv[2]) : 45.0;
    double width = argc > 3 ? atof(argv[3]) : 105.0;
    double lag = argc > 4 ? atof(argv[4]) : 0.055;
    if (!strcmp(scenario,"inverted")) { white = 450; black = 3200; }
    Pif_Init(&adc,&tick,&pwm);
    sample[8] = 2000;
    if (PIF_LESSON == PIF_TRACK) {
        calibrate_and_start();
        if (!strcmp(scenario,"waves")) waves(0,300,lookahead,width,lag);
        else if (!strcmp(scenario,"noise")) waves(1,300,lookahead,width,lag);
        else if (!strcmp(scenario,"tight")) waves(1,200,lookahead,width,lag);
        else signals();
    } else {
        fill(white); frames(16); press();
        if (PIF_LESSON == PIF_SENSORS) assert(!watch.pwm_left && !watch.pwm_right);
        if (PIF_LESSON == PIF_WHEELS) {
            assert(watch.pwm_left == TEST_DUTY && !watch.pwm_right);
            frames(250); press(); assert(!watch.pwm_left && watch.pwm_right == TEST_DUTY);
        }
    }
    now += 24; Pif_Poll();
    assert(watch.state == PIF_ADC_FAULT && !watch.pwm_left && !watch.pwm_right);
    HAL_ADC_ErrorCallback(&adc);
    frames(5); assert(watch.state == PIF_ADC_FAULT);
    printf("PASS lesson %d / %s\n", PIF_LESSON,scenario);
}
