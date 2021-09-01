#pragma once

/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include "util.h"


/**********************************************************************/
// macro define
/**********************************************************************/
#define EN_PIN								PB12
// #define PLUE_PIN           PB15
// #define PLUE_PIN           PB12
#define PLUE_PIN							PD8
#define DIR_PIN								PB14
#define DEFAULT_V							10000


/**********************************************************************/
// type define
/**********************************************************************/
class pluse_gen{
private:
	static pluse_gen* instance;

	/**
	 * sync all channel value to host
	 *  @param:									- none
	 *  @return:               	- none
	 */
	pluse_gen();
	~pluse_gen();

public:
	bool dir;
	long target_pos;							// <! steps counter as position
	long cur_pos;									// <! steps counter as position
	uint32_t velocity;						// <! steps / second
	long pluse_count_one_round;		// <! total pluse in one round
	TIM_HandleTypeDef htim1;			// <! counter for current position
	uint16_t interrupt_counter;

	static pluse_gen* getInstance(void) {
		if(NULL  == instance) {
			instance = new pluse_gen();
		}
		return instance;
	}

	/**
	 * start pwm
	 *  @param 									- none
	 *  @return:               	- none
	 */
	void pwm_pluse_start(void);

	/**
	 * must call this function in the main loop
	 *  @param 									- none
	 *  @return:               	- none
	 */
	void run(void);

	/**
	 * set target pos
	 *  @param t								- target postion 
	 *  @return:               	- none
	 */
	void set_target(long t);

	/**
	 * get cur pos
	 *  @param									- none
	 *  @return:               	- none
	 */
	long get_cur(void);

	/**
	 * set run velocity
	 *  @param v								- setting velocity
	 *  @return:               	- none
	 */
	void set_velocity(uint32_t v);

	/**
	 * reset pluse gen
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void reset(void);

	/**
	 * stop
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void stop(void);

	/**
	 * random walk 
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void random_walk_run(void);

	/**
	 * random walk set
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void set_random_walk(long pluse_count_one_round);

private:
	uint32_t _en_pin;
	uint32_t _dir_pin;
	bool _new_target;
	bool _new_velocity;
	bool _random_walk_flag;					// <! random walk
	long _pluse_count_one_round;		// <! pluse count of one round
	TIM_HandleTypeDef _htim3;				// <! PWM pluse generator
	HardwareTimer*  _hwTimer1;			// <! Arduino Hardware timer
	HardwareTimer*  _hwTimer3;			// <! Arduino Hardware timer
};


/**********************************************************************/
// extern variable
/**********************************************************************/


/**********************************************************************/
// extern function declaretion
/**********************************************************************/

