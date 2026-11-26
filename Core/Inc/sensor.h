#ifndef PIF_SENSOR_H
#define PIF_SENSOR_H
#include <stdbool.h>
#include "stm32f1xx_hal.h"
bool Sensor_Start(ADC_HandleTypeDef *adc, TIM_HandleTypeDef *trigger);
void Sensor_BeginCapture(void);
void Sensor_Capture(bool white);
void Sensor_Normalize(void);
float Sensor_Position(void);
#endif
