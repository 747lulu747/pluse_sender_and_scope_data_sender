/**********************************************************************/
// Include file
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "console.h"
#include "pluse_gen.h"
#include "STM32TimerInterrupt.h"
#include "encoder.h"
#include "pwm_encoder.h"


/**********************************************************************/
// macro define
/**********************************************************************/
#define LED_PIN									PD12
#define PLUSE_COUNT_ROUND				(4000)
#define SENSOR_COUNT_ROUD				(8000)
#define SENSOR_ELEC_COUNT_ROND	(SENSOR_COUNT_ROUD/50)


/**********************************************************************/
// local variable
/**********************************************************************/
// HardwareTimer pwm_gen(TIM9);
console con(Serial);
pluse_gen *pg;

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
// Encoder encoder_1(23, 22);		// PE4 PE6
// Encoder encoder_2(38, 37);		// PC6 PC8
// STM32Timer report_timer(TIM2);
encoder *e1;
encoder *e2;

pwm_encoder *pe1;

/*
use for staticstic
*/
float max_err = 0;

/**
 * angel to DAC
*/
DAC_HandleTypeDef hdac;


/**********************************************************************/
// function declaretion
/**********************************************************************/
void TaskPlue( void *pvParameters );
/**
 * This function will been register to console and will been call when recieve "Txxx".
 * When be called, parse the command line and set the target position
 *  @param 									- none
 *  @return:               	- none
 */
void pg_set_target(const char *cmd_line);
/**
 * This function will been register to console and will been call when recieve "Vxxx".
 * When be called, parse the command line and set the target velocity
 *  @param 									- none
 *  @return:               	- none
 */
void pg_set_velocity(const char *cmd_line);
/**
 * reset the pluse generator
 *  @param 									- none
 *  @return:               	- none
 */
void pg_reset(const char *cmd_line);

/**
 * stop the pluse generator
 *  @param 									- none
 *  @return:               	- none
 */
void pg_stop(const char *cmd_line);

/**
 * random run
 *  @param 									- none
 *  @return:               	- none
 */
void pg_randwom_walk(const char *cmd_line);

/**
 * output pluse generat information
 *  @param 									- none
 *  @return:               	- none
 */
void pg_info_print(const char *cmd_line);

/**
 * Add one step
 *  @param 									- none
 *  @return:               	- none
 */
void pg_add(const char *cmd_line);

/**
 * Sub one step
 *  @param 									- none
 *  @return:               	- none
 */
void pg_sub(const char *cmd_line);

/**
 * report data upload
 *  @param 									- none
 *  @return:               	- none
 */
void report(void);

/**
 * encoder to pwm: 
 *  @param:pg 							- pwm generator
 *  @param:e 								- encoder
 *  @return:               	- none
 */
void encoder_to_pwm(HardwareTimer &p, encoder *e);

/**
 * encoder to DAC: 
 *  @param:pg 							- pwm generator
 *  @param:e 								- encoder
 *  @return:               	- none
 */
void encoder_to_dac(DAC_HandleTypeDef &p, encoder *e);


/**********************************************************************/
// function define
/**********************************************************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
	Serial.println("startup");

	pg = pluse_gen::getInstance();
	e1 = encoder::get_encoder_1();
	e2 = encoder::get_encoder_2();

	pe1 = pwm_encoder::get_pwm_encoder_1();
	pe1->start();
	
	// register the call back function
	// set target position and velocity
	con.register_cmd('T', pg_set_target);
	con.register_cmd('V', pg_set_velocity);
	con.register_cmd('R', pg_reset);
	con.register_cmd('S', pg_stop);
	con.register_cmd('W', pg_randwom_walk);
	con.register_cmd('?', pg_info_print);
	con.register_cmd('+', pg_add);
	con.register_cmd('-', pg_sub);
	con.set_scope_agency_flag(console::SIMPLE_SCOPE_FLAG);
	con.start();

	/**
	 * angle to DAC
	*/
	DAC_ChannelConfTypeDef sConfig = {0};
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_DAC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**DAC GPIO Configuration
	PA4     ------> DAC_OUT1
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Main loop task
  xTaskCreate(
    TaskPlue
    ,  "Plue"  								// A name just for humans
    ,  1024     							// This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL     							// 
    ,  1   										// Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  // start scheduler
  vTaskStartScheduler();
}

long oldPosition  = -999;
void loop() {
  // put your main code here, to run repeatedly:
}

void TaskPlue(void *pvParameters)  {
  (void) pvParameters;

	// report_timer.attachInterrupt(100, report);
  for (;;) {
		// report();
		// pg->run();
		// vTaskDelay(configTICK_RATE_HZ/500);
		// encoder_to_pwm(NULL, e2);
		// e_c = 50 * e->read() % SENSOR_COUNT_ROUD;
		// encoder_to_dac(hdac, e2);
		Serial.print("pe1 ");
		Serial.println(pe1->_value);
		vTaskDelay(configTICK_RATE_HZ/500);
  }
}

/**
 * This function will been register to console and will been call when recieve "Txxx".
 * When be called, parse the command line and set the target position
 *  @param 									- none
 *  @return:               	- none
 */
void pg_set_target(const char *cmd_line) {
	long t;
	if(1 == sscanf(cmd_line, "%d", &t)) {
		Serial.print("set target pos to ");
		Serial.println(t);
		pg->set_target(t);
	}
}
/**
 * This function will been register to console and will been call when recieve "Vxxx".
 * When be called, parse the command line and set the target velocity
 *  @param 									- none
 *  @return:               	- none
 */
void pg_set_velocity(const char *cmd_line) {
	uint32_t v;
	if(1 == sscanf(cmd_line, "%d", &v)) {
		Serial.print("set target volocity to ");
		Serial.println(v);
		pg->set_velocity(v);
	}
}

/**
 * reset the pluse generator
 * When be called, parse the command line and set the target velocity
 *  @param 									- none
 *  @return:               	- none
 */
void pg_reset(const char *cmd_line) {
	Serial.println("reset");
	max_err = 0;
	pg->reset();
	e1->set(0);
	e2->set(0);
}

/**
 * reset the pluse generator
 * When be called, parse the command line and set the target velocity
 *  @param 									- none
 *  @return:               	- none
 */
void pg_stop(const char *cmd_line) {
	pg->stop();
	pg_info_print(nullptr);
}

/**
 * reset the pluse generator
 * When be called, parse the command line and set the target velocity
 *  @param 									- none
 *  @return:               	- none
 */
void pg_randwom_walk(const char *cmd_line) {
	if('\0' == cmd_line[0]){
		Serial.println("random walk");
		pg->set_random_walk(PLUSE_COUNT_ROUND);
	}
	else {
		Serial.println("random walk error command");
	}
}

/**
 * output pluse generat information
 *  @param 									- none
 *  @return:               	- none
 */
void pg_info_print(const char *cmd_line) {
	Serial.print("target: ");
	Serial.print(pg->target_pos);

	Serial.print("\tcurrent: ");
	Serial.print(pg->cur_pos);

	Serial.print("\tvelocity: ");
	Serial.print(pg->velocity);
}

/**
 * Add step
 *  @param 									- none
 *  @return:               	- none
 */
void pg_add(const char *cmd_line) {
	int add;
	if(1 == sscanf(cmd_line, "%d", &add)) {
		Serial.print("go+ ");
		Serial.println(add);
		pg->set_target(pg->target_pos + add);
	}
	else{
		Serial.println("go+ 1");
		pg->set_target(pg->target_pos + 1);
	}
}

/**
 * Sub step
 *  @param 									- none
 *  @return:               	- none
 */
void pg_sub(const char *cmd_line) {
	int add;
	if(1 == sscanf(cmd_line, "%d", &add)) {
		Serial.print("go- ");
		Serial.println(add);
		pg->set_target(pg->target_pos - add);
	}
	else{
		Serial.println("go- 1");
		pg->set_target(pg->target_pos - 1);
	}
}

/**
 * report timer cb
 *  @param 									- none
 *  @return:               	- none
 */
void report(void) {
	float target;
	float cur_encoder_1;
	float delta;
	
	target = pg->get_cur();
	cur_encoder_1 = (float)e1->read() * PLUSE_COUNT_ROUND / SENSOR_COUNT_ROUD;
	delta = target - cur_encoder_1;

	if(max_err < abs(delta)) {
		max_err = abs(delta);
	}

	con.set_channel_value(console::CH_1, target);
	con.set_channel_value(console::CH_2, cur_encoder_1);
	con.set_channel_value(console::CH_3, delta);
	con.set_channel_value(console::CH_4, max_err);
	con.sync(1<<0 | 1<<1 | 1<<2 | 1<<3);
}

/**
 * encoder to pwm: 
 *  @param:pg 							- pwm generator
 *  @param:e 								- encoder
 *  @return:               	- none
 */
void encoder_to_pwm(HardwareTimer &p, encoder *e) {
	static uint32_t e_c = 0;
	
	e_c = 50 * e->read() % SENSOR_COUNT_ROUD;
	p.setPWM(1, PE5, 10000, 50 * 100 * (float)e_c / SENSOR_COUNT_ROUD);
}

/**
 * encoder to DAC: 
 *  @param:p 								- dac generator
 *  @param:e 								- encoder
 *  @return:               	- none
 */
void encoder_to_dac(DAC_HandleTypeDef &p, encoder *e) {
	HAL_DAC_SetValue(&p, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 4096 * (float)e->read() / SENSOR_COUNT_ROUD );
	HAL_DAC_Start(&p,DAC_CHANNEL_1);
}
