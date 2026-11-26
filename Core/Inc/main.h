#ifndef MAIN_H
#define MAIN_H
#include "stm32f1xx_hal.h"
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timer);
void Error_Handler(void);
#endif
