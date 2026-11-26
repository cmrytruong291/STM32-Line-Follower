#pragma once
#include <stdint.h>
typedef struct { struct { uint32_t ScanConvMode, NbrOfConversion, ExternalTrigConv; } Init; } ADC_HandleTypeDef;
typedef struct {uint32_t arr, out[2];} TIM_HandleTypeDef;
typedef int GPIO_PinState;
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define GPIOB 1
#define GPIOC 2
#define GPIO_PIN_10 (1<<10)
#define GPIO_PIN_12 (1<<12)
#define GPIO_PIN_13 (1<<13)
#define GPIO_PIN_14 (1<<14)
#define GPIO_PIN_15 (1<<15)
#define TIM_CHANNEL_3 0
#define TIM_CHANNEL_4 1
#define HAL_OK 0
#define __HAL_TIM_GET_AUTORELOAD(m) ((m)->arr)
#define __HAL_TIM_SET_COMPARE(m,c,v) ((m)->out[c]=(v))
#define __disable_irq() ((void)0)
#define __get_PRIMASK() 0U
#define __set_PRIMASK(k) ((void)(k))
#define __WFI() ((void)0)
uint32_t HAL_GetTick(void);
int HAL_GPIO_ReadPin(int,int);
void HAL_GPIO_WritePin(int,int,int);
int HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef*);
int HAL_TIM_PWM_Start(TIM_HandleTypeDef*,unsigned);
int HAL_ADC_Start_DMA(ADC_HandleTypeDef*,uint32_t*,uint32_t);
int HAL_TIM_Base_Start(TIM_HandleTypeDef*);

#define ADC_SCAN_DISABLE 0
#define ADC_SOFTWARE_START 0
#define ADC_REGULAR_RANK_1 1
#define ADC_SAMPLETIME_71CYCLES_5 71
#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1
#define ADC_CHANNEL_2 2
#define ADC_CHANNEL_3 3
#define ADC_CHANNEL_4 4
#define ADC_CHANNEL_5 5
#define ADC_CHANNEL_6 6
#define ADC_CHANNEL_7 7
#define ADC_CHANNEL_8 8
typedef struct { uint32_t Channel, Rank, SamplingTime; } ADC_ChannelConfTypeDef;
int HAL_ADC_Init(ADC_HandleTypeDef *);
int HAL_ADC_ConfigChannel(ADC_HandleTypeDef *, ADC_ChannelConfTypeDef *);
int HAL_ADC_Start(ADC_HandleTypeDef *);
int HAL_ADC_Stop(ADC_HandleTypeDef *);
int HAL_ADC_PollForConversion(ADC_HandleTypeDef *, uint32_t);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *);
