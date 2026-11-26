#include "sensor.h"
#include "controller.h"
#include "filter.h"

static ADC_HandleTypeDef *input_adc;
static uint16_t samples[18] __attribute__((aligned(4)));

bool Sensor_Start(ADC_HandleTypeDef *adc, TIM_HandleTypeDef *trigger) {
    input_adc = adc;
    if (HAL_ADCEx_Calibration_Start(adc) != HAL_OK) return false;
    if (HAL_ADC_Start_DMA(adc, (uint32_t *)(void *)samples, 18U) != HAL_OK) return false;
    return HAL_TIM_Base_Start(trigger) == HAL_OK;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *adc) {
    if (adc == input_adc) Controller_Frame(samples);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc) {
    if (adc == input_adc) Controller_Frame(samples + 9);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *adc) {
    if (adc == input_adc) Controller_Fault();
}

static uint32_t sums[8];
static unsigned capture_count;
void Sensor_BeginCapture(void) {
    for (unsigned i = 0; i < 8; ++i) sums[i] = 0;
    capture_count = 0;
}

void Sensor_Capture(bool white) {
    for (unsigned i = 0; i < 8; ++i) sums[i] += watch.adc[i];
    if (++capture_count < CAL_SAMPLES) return;
    watch.bad_mask = 0;
    for (unsigned i = 0; i < 8; ++i) {
        uint16_t mean = (uint16_t)(sums[i] / CAL_SAMPLES);
        if (white) watch.white[i] = mean;
        else {
            watch.black[i] = mean;
            int span = (int)mean - watch.white[i];
            if (span > -MIN_SPAN && span < MIN_SPAN) watch.bad_mask |= (1U << i);
        }
    }
    Filter_Reset();
    watch.state = white ? PIF_WAIT_BLACK : (watch.bad_mask ? PIF_CAL_BAD : PIF_READY);
}


void Sensor_Normalize(void) {
    for (unsigned i = 0; i < 8; ++i) {
        uint16_t sample = Filter_Sample(i, watch.adc[i]);
        int span = (int)watch.black[i] - watch.white[i];
        /* Called only after a successful white/black calibration. */
        int q = 1000 * ((int)sample - watch.white[i]) / span;
        watch.q[i] = (uint16_t)(q < 0 ? 0 : (q > 1000 ? 1000 : q));
    }
    Filter_EndFrame();
}
/* Remove a common background after checking for a broad black timing bar.
   Two separated strong dark groups are ambiguous: use bounded gap handling. */
float Sensor_Position(void) {
    unsigned dark = 0, floor = 1000, next_floor = 1000, peak = 0;
    for (unsigned i = 0; i < 8; ++i) {
        unsigned q = watch.q[i];
        if (q <= floor) {
            next_floor = floor;
            floor = q;
        } else if (q < next_floor) {
            next_floor = q;
        }
        if (q > peak) peak = q;
        dark += q >= 750U;
    }
    watch.contrast = (uint16_t)(peak - next_floor);
    watch.wide = dark >= 7U;
    if (watch.wide) {
        watch.line_detected = 1U;
        watch.error_mm = 0;
        return 0;
    }

    /* Weighted centroid of the strongest contiguous group only. */
    unsigned mass = 0, best = 0, second = 0;
    int moment = 0, best_moment = 0;
    for (unsigned i = 0; i <= 8; ++i) {
        int weight = i < 8 ? (int)watch.q[i] - (int)next_floor - 80 : 0;
        if (weight > 0) {
            mass += (unsigned)weight;
            moment += (2*(int)i - 7)*weight;
        } else {
            if (mass > best) {
                second = best;
                best = mass;
                best_moment = moment;
            } else if (mass > second) {
                second = mass;
            }
            mass = 0;
            moment = 0;
        }
    }
    watch.line_detected = watch.contrast >= 260U && best >= 340U && 2U*second < best;
    float error = watch.line_detected ? best_moment * (SENSOR_MM * 0.5f) / best : 0;
    watch.error_mm = (int16_t)error;
    return error;
}
