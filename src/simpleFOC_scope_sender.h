#pragma once
/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>


/**********************************************************************/
// macro define
/**********************************************************************/


/**********************************************************************/
// type define
/**********************************************************************/
class simpleFOC_scope_sender
{
public:
	enum {
		CH_1 = 0,
		CH_2,
		CH_3,
		CH_4,
		CH_5,
		CH_6,
		CH_7,
		CH_MAX,
	};

	float ch_v[CH_MAX];

	/**
	 * Update a channel value
	 *  @param ch					 			- which channel
	 *  @param v								- channel value
	 *  @return:               	- none
	 */
	void set_channel_vale(uint8_t ch, float v);

	/**
	 * Update a channel value
	 *  @param len				 			- output lenght
	 *  @param buf							- output buffer
	 *  @return:               	- none
	 */
	void sync(uint32_t *out_len, uint8_t *buf, uint8_t ch_out_maskbit = 0xFF);
};

/**********************************************************************/
// extern function declaretion
/**********************************************************************/

