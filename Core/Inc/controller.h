#ifndef PIF_CONTROLLER_H
#define PIF_CONTROLLER_H
#include "config.h"
#include "stm32f1xx_hal.h"
typedef enum {
    PIF_IDLE, PIF_LEFT_TEST, PIF_RIGHT_TEST,
    PIF_WAIT_WHITE, PIF_READ_WHITE, PIF_WAIT_BLACK, PIF_READ_BLACK,
    PIF_READY, PIF_SEEK_START, PIF_LEAVE_START, PIF_RUNNING,
    PIF_FINISHED, PIF_LOST, PIF_CAL_BAD, PIF_ADC_FAULT
} PifState;

/* Live Expressions: watch. Reading is optional for autonomous operation. */
typedef struct {
    uint32_t frames;
    uint16_t adc[9], white[8], black[8], q[8];
    uint16_t pwm_left, pwm_right;  /* per mille, 0..1000 */
    uint16_t pin_mV;              /* whole pack, assumes VDDA=3300 mV */
    int16_t error_mm;
    uint8_t line_detected, wide, bad_mask;
    uint16_t pwm_base, contrast, gap_frames;
    uint32_t gap_events;          /* gaps since the last start press */
    PifState state;
} PifWatch;
extern volatile PifWatch watch;

void Pif_Init(ADC_HandleTypeDef *adc, TIM_HandleTypeDef *trigger, TIM_HandleTypeDef *pwm);
void Pif_Poll(void);
void Controller_Frame(const uint16_t *src);
void Controller_Fault(void);
#endif
