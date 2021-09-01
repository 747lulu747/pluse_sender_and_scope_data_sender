/**********************************************************************/
// Include file
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "console.h"
#include "pluse_gen.h"
#include "STM32TimerInterrupt.h"
#include "encoder.h"


/**********************************************************************/
// macro define
/**********************************************************************/
#define LED_PIN								PD12
#define PLUSE_COUNT_ROUND			(8000)
#define SENSOR_COUNT_ROUD			(8000)


/**********************************************************************/
// local variable
/**********************************************************************/
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
		report();
		pg->run();
		vTaskDelay(configTICK_RATE_HZ/100);
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
	int32_t cur_encoder_1, last_encoder_1 = 0;
	int32_t cur_encoder_2, last_encoder_2 = 0;
	
	// Target
	// con.set_channel_value(console::CH_1, pg->cur_pos);
	con.set_channel_value(console::CH_1, pg->get_cur());

	// encoder 1
	cur_encoder_1 = e1->read();
	// con.set_channel_value(console::CH_2, cur_encoder_1 * PLUSE_COUNT_ROUND / SENSOR_COUNT_ROUD);
	con.set_channel_value(console::CH_2, cur_encoder_1);
	// con.set_channel_value(console::CH_3, (float)(cur_encoder_1 - last_encoder_1) * 100 / 8000);
	// last_encoder_1 = cur_encoder_1;
	
	// encoder 2
	// cur_encoder_2 = -encoder_2.read();
	cur_encoder_2 = e2->read();
	// con.set_channel_value(console::CH_2, cur_encoder_2 * PLUSE_COUNT_ROUND / SENSOR_COUNT_ROUD);
	con.set_channel_value(console::CH_3, cur_encoder_2);
	// con.set_channel_value(console::CH_5, (float)(cur_encoder_2 - last_encoder_2) * 100 / 8000);
	last_encoder_2 = cur_encoder_2;

	con.sync(1<<0 | 1<<1 | 1<<2);
}

// if(current_pos == target_pule_cnt)
// {
// 	// 随机产生速度、方向、位置
// 	srand(xTaskGetTickCount());
// 	dir = rand() % 2;
// 	target_pule_cnt = rand() % (ONE_CIRCLE_PLUE);
// 	plue_fre = rand() % MAX_FRQ;
// 	if(0 == plue_fre || plue_fre < 1000)
// 	{
// 		plue_fre = 1000;
// 	}
// 	Serial.print("DIR ");
// 	Serial.print(dir);
// 	Serial.print("\tTARGET POS ");
// 	Serial.print(target_pos);
// 	Serial.print("\tSPEED ");
// 	Serial.println(plue_fre);
// 	current_pos = 0;
// 	if(dir)
// 	{
// 		digitalWrite(DIR_PIN, HIGH);
// 	}
// 	else
// 	{
// 		digitalWrite(DIR_PIN, LOW);
// 	}
// }

