/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include "simpleFOC_scope_sender.h"


/**********************************************************************/
// extern variable declaretion
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/


/**********************************************************************/
// function definition
/**********************************************************************/
/**
 * Update a channel value
 *  @param ch					 			- which channel
 *  @param v								- channel value
 *  @return:               	- none
 */
void simpleFOC_scope_sender::set_channel_vale(uint8_t ch, float v) {
	if(ch >= CH_MAX)
		return;

	ch_v[ch] = v;
}

/**
 * Update a channel value
 *  @param len				 			- output lenght
 *  @param buf							- output buffer
 *  @return:               	- none
 */
void simpleFOC_scope_sender::sync(uint32_t *out_len, uint8_t *buf, uint8_t ch_out_maskbit) {
	
	*out_len = 0;
	for(uint8_t i = 0; i < CH_MAX; i++) {
		if(ch_out_maskbit & (1<<i)) {
			*out_len += sprintf((char *)(buf+(*out_len)), "%f\t", ch_v[i]);
		}
	}
	*out_len += sprintf((char *)(buf+(*out_len)), "\n");

}
