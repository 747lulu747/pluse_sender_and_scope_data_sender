/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "pluse_gen.h"


/**********************************************************************/
// macro define
/**********************************************************************/


/**********************************************************************/
// typedef
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/
// init static member
pluse_gen* pluse_gen::instance = NULL;


/**********************************************************************/
// function define
/**********************************************************************/
extern "C" {
	/**
		* @brief  Period elapsed callback in non-blocking mode
		* @param  htim TIM handle
		* @retval None
		*/
	void pluse_gen_update_callback(void) {

		// Serial.println("pluse_gen_update_callback");
		pluse_gen *instance = pluse_gen::getInstance();

		if(instance->dir)
			instance->cur_pos += (uint16_t)instance->interrupt_counter;
		else
			instance->cur_pos -= (uint16_t)instance->interrupt_counter;

		if(instance->cur_pos == instance->target_pos) {
			instance->stop();
		}
		else {
			long abs_delta;
			abs_delta = abs(instance->target_pos - instance->cur_pos);
			if(abs_delta > 0xFFFF) {
				instance->interrupt_counter = 0xFFFF;
			}
			else {
				instance->interrupt_counter = abs_delta & 0xFFFF;
			}
			__HAL_TIM_SET_AUTORELOAD(&instance->htim1, instance->interrupt_counter);
		}
	}
}

/**
 * sync all channel value to host
 *  @param en_pin						- enable pin
 *  @param dir_pin					- dir pin
 *  @param step_pin					- step pin
 *  @return:               	- none
 */
pluse_gen::pluse_gen() 
{
	_en_pin = EN_PIN;
	_dir_pin = DIR_PIN;

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(_en_pin, OUTPUT);
	pinMode(_dir_pin, OUTPUT);

	digitalWrite(LED_BUILTIN, LOW);
	digitalWrite(_en_pin, HIGH);
	digitalWrite(_dir_pin, HIGH);

	velocity = DEFAULT_V;
	_new_target = false;
	_new_velocity = false;
	_random_walk_flag = false;
	cur_pos = 0;

	_hwTimer1 = new HardwareTimer(TIM1);
	_hwTimer3 = new HardwareTimer(TIM3);

	/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
 	{
		// __HAL_RCC_TIM1_CLK_ENABLE();

		TIM_SlaveConfigTypeDef sSlaveConfig = {0};
		TIM_MasterConfigTypeDef sMasterConfig = {0};

		htim1.Instance = TIM1;
		htim1.Init.Prescaler = 0;
		htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim1.Init.Period = 65535;
		htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim1.Init.RepetitionCounter = 0;
		htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		HAL_TIM_Base_Init(&htim1);
		
		sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
		sSlaveConfig.InputTrigger = TIM_TS_ITR2;
		HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig);

		/*
		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  	HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
		*/

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

		_hwTimer1->attachInterrupt(pluse_gen_update_callback);
	}

	/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
	{
		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		TIM_MasterConfigTypeDef sMasterConfig = {0};
		TIM_OC_InitTypeDef sConfigOC = {0};

		_htim3.Instance = TIM3;
		_htim3.Init.Prescaler = 0;
		_htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
		_htim3.Init.Period = 4294967295;
		_htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		_htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		HAL_TIM_Base_Init(&_htim3);

		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		HAL_TIM_ConfigClockSource(&_htim3, &sClockSourceConfig);
		HAL_TIM_PWM_Init(&_htim3);

		sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
		HAL_TIMEx_MasterConfigSynchronization(&_htim3, &sMasterConfig);

		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = 0;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		HAL_TIM_PWM_ConfigChannel(&_htim3, &sConfigOC, TIM_CHANNEL_1);

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

/**
 * start pwm
 *  @param 									- none
 *  @return:               	- none
 */
void pluse_gen::pwm_pluse_start(void) {
	/**
	 * when the cur_position == target_position
	 * an interrupt happen.
	*/
	long abs_delta;

	/*
	 * stop timer 1 and read the counter to current positon 
	*/
	__HAL_TIM_DISABLE(&htim1);
	if(digitalRead(instance->_dir_pin))
		 cur_pos += (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
	else
		 cur_pos -= (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
	__HAL_TIM_SET_COUNTER(&htim1, 0);

	if(target_pos == cur_pos) {
		return;
	}
	else if(target_pos > cur_pos) {
		htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
		digitalWrite(_dir_pin, HIGH);
		dir = true;
	}
	else {
		htim1.Init.CounterMode = TIM_COUNTERMODE_DOWN;
		digitalWrite(_dir_pin, LOW);
		dir = false;
	}

	abs_delta = abs(target_pos - cur_pos);
	if(abs_delta > 0xFFFF) {
		interrupt_counter = 0xFFFF;
	}
	else {
		interrupt_counter = abs_delta & 0xFFFF;
	}


	/**
	 * setting timer1
	*/
	// htim1.Init.Prescaler = 0;
	// htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	// htim1.Init.Period = interrupt_counter;
	// htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	// htim1.Init.RepetitionCounter = 0;
	// htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	// HAL_TIM_Base_Init(&htim1);
	__HAL_TIM_SET_AUTORELOAD(&htim1, interrupt_counter);
  
	// HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
  // HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	HAL_TIM_Base_Start_IT(&htim1);

	/**
	 * setting timer5
	*/
  _htim3.Init.Period = _hwTimer3->getTimerClkFreq() / velocity;
  HAL_TIM_Base_Init(&_htim3);
	TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = _htim3.Init.Period / 2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&_htim3, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&_htim3, TIM_CHANNEL_1);
}

/**
 * must call this function in the main loop
 *  @param 								- none
 *  @return:          		- none
 */
void pluse_gen::run(void) {

	if(target_pos == cur_pos) {
		if(_new_target){
			Serial.print("current_pos ");
			Serial.println(cur_pos);
			_new_target = 0;
		}
	}

	if(_random_walk_flag) {
		random_walk_run();
	}
}

/**
 * set target pos
 *  @param t								- target postion 
 *  @return:               	- none
 */
void pluse_gen::set_target(long t) {
	
	if(t == target_pos){
		return;
	}

	target_pos = t;
	_new_target = true;
	_random_walk_flag = false;	

	if(target_pos == cur_pos) {
		return;
	}

	pwm_pluse_start();
}

/**
 * get cur pos
 *  @param									- none
 *  @return:               	- none
 */
long pluse_gen::get_cur(void) {
	long ret;
	uint32_t counter;

	counter = __HAL_TIM_GET_COUNTER(&htim1);
	if(dir)
		ret = cur_pos + counter;
	else 
		ret = cur_pos - counter;

	return ret;
}

/**
 * set run velocity
 *  @param v								- setting velocity
 *  @return:               	- none
 */
void pluse_gen::set_velocity(uint32_t v) {
	velocity = v;
	_new_velocity = true;
	pwm_pluse_start();
	_new_velocity = false;
}

/**
 * reset pluse gen
 *  @param :								- none
 *  @return:               	- none
 */
void pluse_gen::reset(void) {
	_random_walk_flag = false;
	target_pos = cur_pos = 0;
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	digitalWrite(_dir_pin, HIGH);
	HAL_TIM_PWM_Stop(&_htim3, TIM_CHANNEL_1);
}

/**
 * reset pluse gen
 *  @param :								- none
 *  @return:               	- none
 */
void pluse_gen::stop(void) {
	HAL_TIM_PWM_Stop(&_htim3, TIM_CHANNEL_1);
}

/**
 * random walk 
 *  @param :								- none
 *  @return:               	- none
 */
void pluse_gen::random_walk_run(void) {
	long t;

	if(cur_pos == target_pos) {
		srand(xTaskGetTickCount());
		t = rand() % (5 * pluse_count_one_round);
		t = rand() % 2 ? t : -t;
		target_pos = t;
		pwm_pluse_start();
	}
}

/**
 * random walk set
 *  @param :								- none
 *  @return:               	- none
 */
void pluse_gen::set_random_walk(long pcor) {
	pluse_count_one_round = pcor;
	_random_walk_flag = true;
	HAL_TIM_PWM_Start(&_htim3, TIM_CHANNEL_1);
}

