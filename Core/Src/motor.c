#include "motor.h"
#include "config.h"

static TIM_HandleTypeDef *motor_timer;

void Motor_SetDuty(uint16_t left, uint16_t right) {
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(motor_timer) + 1U;
    if (left > WHEEL_DUTY_CAP) left = WHEEL_DUTY_CAP;
    if (right > WHEEL_DUTY_CAP) right = WHEEL_DUTY_CAP;
    __HAL_TIM_SET_COMPARE(motor_timer, TIM_CHANNEL_3, period * left / 1000U);
    __HAL_TIM_SET_COMPARE(motor_timer, TIM_CHANNEL_4, period * right / 1000U);
}

bool Motor_Init(TIM_HandleTypeDef *timer) {
    motor_timer = timer;
    Motor_SetDuty(0, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15 | GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14 | GPIO_PIN_13, GPIO_PIN_SET);
    return HAL_TIM_PWM_Start(timer, TIM_CHANNEL_3) == HAL_OK &&
           HAL_TIM_PWM_Start(timer, TIM_CHANNEL_4) == HAL_OK;
}
