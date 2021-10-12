#pragma once

/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include "util.h"


/**********************************************************************/
// macro define
/**********************************************************************/


/**********************************************************************/
// type define
/**********************************************************************/


/**********************************************************************/
// extern variable
/**********************************************************************/


/**********************************************************************/
// extern function declaretion
/**********************************************************************/
class pwm_encoder {

public:
	pwm_encoder(int pwm_to_encoder_scale, TIM_TypeDef *tim);
	void start(void);
	uint32_t read(void);

	static pwm_encoder* pwm_e1;
	static pwm_encoder* pwm_e2;
	static pwm_encoder* pwm_e3;

	static pwm_encoder* get_pwm_encoder_1(void);
	static pwm_encoder* get_pwm_encoder_2(void);
	static pwm_encoder* get_pwm_encoder_3(void);

	TIM_TypeDef *_tim;
	TIM_HandleTypeDef _htim;
	volatile uint32_t _tmp1;
	volatile uint32_t _tmp2;
	uint32_t _value;
	int _pwm_to_encoder_scale;

private:
	// 
};
