/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "encoder.h"


/**********************************************************************/
// macro define
/**********************************************************************/


/**********************************************************************/
// typedef
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/
encoder* encoder::e1 = new encoder(TIM5);
encoder* encoder::e2 = new encoder(TIM2);


/**********************************************************************/
// function define
/**********************************************************************/
encoder::encoder(TIM_TypeDef *tim) {
	_tim = tim;

	if (TIM2 == _tim) {
		GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA5     ------> TIM2_CH1
    PB3     ------> TIM2_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

	else if(TIM5 == _tim) {
		GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_TIM5_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM5 GPIO Configuration
    PA0-WKUP     ------> TIM5_CH1
    PA1     ------> TIM5_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else {
		while(1);
	}

	TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  _htim.Instance = _tim;
  _htim.Init.Prescaler = 0;
  _htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  _htim.Init.Period = 4294967295;
  _htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  _htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 2;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 2;
  HAL_TIM_Encoder_Init(&_htim, &sConfig);
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&_htim, &sMasterConfig);
	HAL_TIM_Encoder_Start(&_htim, TIM_CHANNEL_ALL);
}

encoder* encoder::get_encoder_1(void) {
	return e1;
}
encoder* encoder::get_encoder_2(void) {
	return e2;
}

long encoder::read(void) {
	_cur_pos = __HAL_TIM_GET_COUNTER(&_htim);
	return _cur_pos;
}

void encoder::set(long v) {
	__HAL_TIM_SET_COUNTER(&_htim, v);
}

