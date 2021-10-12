/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "pwm_encoder.h"


/**********************************************************************/
// macro define
/**********************************************************************/
#define PWM_TO_E_SCALE		(51200)


/**********************************************************************/
// typedef
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/
pwm_encoder* pwm_encoder::pwm_e1 = new pwm_encoder(PWM_TO_E_SCALE, TIM2);
pwm_encoder* pwm_encoder::pwm_e2 = new pwm_encoder(PWM_TO_E_SCALE, TIM4);
pwm_encoder* pwm_encoder::pwm_e3 = new pwm_encoder(PWM_TO_E_SCALE, TIM8);


HardwareTimer *_hwTimer2 = new HardwareTimer(TIM2);
HardwareTimer *_hwTimer4 = new HardwareTimer(TIM4);
HardwareTimer *_hwTimer8 = new HardwareTimer(TIM8);

/**********************************************************************/
// function define
/**********************************************************************/
pwm_encoder* pwm_encoder::get_pwm_encoder_1(void){
	return pwm_e1;
}
	pwm_encoder* pwm_encoder::get_pwm_encoder_2(void){
	return pwm_e2;
}
	pwm_encoder* pwm_encoder::get_pwm_encoder_3(void){
	return pwm_e3;
}

pwm_encoder::pwm_encoder(int pwm_to_encoder_scale, TIM_TypeDef *tim) {
	_tim = tim;
	_tmp1 = _tmp2 = 0;
	_pwm_to_encoder_scale = pwm_to_encoder_scale;

	GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim==TIM1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PE9     ------> TIM1_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  }
  else if(tim==TIM2)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA5     ------> TIM2_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  }
  else if(tim==TIM4){
    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  }
  else if(tim==TIM8)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM8_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**TIM8 GPIO Configuration
    PC6     ------> TIM8_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* TIM8 interrupt Init */
    HAL_NVIC_SetPriority(TIM8_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
  }

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

	_htim.Instance = _tim;
  _htim.Init.Prescaler = 0;
  _htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	if(_htim.Instance == TIM1 || _htim.Instance == TIM8)
  	_htim.Init.Period = 65535;
	else
		_htim.Init.Period = 4294967295;
  _htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  _htim.Init.RepetitionCounter = 0;
  _htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&_htim) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&_htim, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&_htim) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&_htim, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&_htim, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&_htim, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&_htim, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

void pwm_encoder::start(void) {
	HAL_TIM_IC_Start_IT(&_htim, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&_htim, TIM_CHANNEL_2);
}

uint32_t pwm_encoder::read(void) {
	return _value;
}

void pwm_encoder_HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
// void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	pwm_encoder* e;
	if(htim->Instance == TIM2)
		e = pwm_encoder::pwm_e1;
	else if(htim->Instance == TIM4)
		e = pwm_encoder::pwm_e2;
	else if(htim->Instance == TIM8)
		e = pwm_encoder::pwm_e3;
	else
		return;

	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		e->_tmp1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);	//周期
	}
	else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		e->_tmp2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);	//占空比
	}

	if(e->_tmp1){
		e->_value = e->_tmp2 * e->_pwm_to_encoder_scale / e->_tmp1;
		// e->_value = e->_tmp2 * 100.0f / e->_tmp1 + 0.5f;
	}
}
