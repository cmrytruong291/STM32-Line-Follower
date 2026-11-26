#ifndef PIF_MOTOR_H
#define PIF_MOTOR_H
#include <stdbool.h>
#include "stm32f1xx_hal.h"
bool Motor_Init(TIM_HandleTypeDef *timer);
void Motor_SetDuty(uint16_t left, uint16_t right);
#endif
