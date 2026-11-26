/* Application entry for the Blue Pill. STM32CubeMX generates the HAL,
   startup, DMA interrupt handler and MSP code from PIF_F103.ioc. */
#include "main.h"
/* USER CODE BEGIN Includes */
#include "controller.h"
/* USER CODE END Includes */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim3, htim4;

static void require_ok(HAL_StatusTypeDef status) {
    if (status != HAL_OK) Error_Handler();
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSIState = RCC_HSI_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    require_ok(HAL_RCC_OscConfig(&osc));

    RCC_ClkInitTypeDef clocks = {0};
    clocks.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV2;
    clocks.APB2CLKDivider = RCC_HCLK_DIV1;
    require_ok(HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_2));

    RCC_PeriphCLKInitTypeDef adc_clock = {0};
    adc_clock.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    adc_clock.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    require_ok(HAL_RCCEx_PeriphCLKConfig(&adc_clock));
}

static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    const uint16_t direction = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_WritePin(GPIOB, direction, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Pin = direction;
    HAL_GPIO_Init(GPIOB, &gpio);
    gpio.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &gpio);
    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);
}

static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void MX_ADC1_Init(void) {
    static const uint32_t channels[] = {
        ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3, ADC_CHANNEL_4,
        ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_8
    };
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 9;
    require_ok(HAL_ADC_Init(&hadc1));
    ADC_ChannelConfTypeDef channel = {0};
    channel.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    for (unsigned i = 0; i < 9; ++i) {
        channel.Channel = channels[i];
        channel.Rank = i + 1;
        require_ok(HAL_ADC_ConfigChannel(&hadc1, &channel));
    }
}

static void MX_TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    require_ok(HAL_TIM_Base_Init(&htim3));
    TIM_MasterConfigTypeDef master = {0};
    master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    require_ok(HAL_TIMEx_MasterConfigSynchronization(&htim3, &master));
}

static void MX_TIM4_Init(void) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    htim4.Instance = TIM4;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 7199;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    require_ok(HAL_TIM_PWM_Init(&htim4));
    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    require_ok(HAL_TIM_PWM_ConfigChannel(&htim4, &oc, TIM_CHANNEL_3));
    require_ok(HAL_TIM_PWM_ConfigChannel(&htim4, &oc, TIM_CHANNEL_4));
    HAL_TIM_MspPostInit(&htim4);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    /* USER CODE BEGIN 2 */
    Pif_Init(&hadc1, &htim3, &htim4);
    /* USER CODE END 2 */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        Pif_Poll();
    }
    /* USER CODE END 3 */
}

void Error_Handler(void) {
    __disable_irq();
    if (htim4.Instance == TIM4) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
    }
    while (1) { }
}
