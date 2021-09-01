/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include "wf_scope_sender.h"


/**********************************************************************/
// extern variable declaretion
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/
const uint32_t wf_scope_sender::BUF_SIZE;
const uint32_t wf_scope_sender::HEADER;
const uint32_t wf_scope_sender::CH_MAX;


/**********************************************************************/
// function definition
/**********************************************************************/
/**
 * build a stream package frome the message
 *  @param m						 		- msg poiter for output
 *  @param out_pack_len			- the output stream package lenght
 *  @param out_pack_buf			- the output stream package buffer
 *  @return:               	- none
 */
void wf_scope_sender::msg_to_pack(struct msg* m, uint32_t *out_pack_len, uint8_t *out_pack_buf) {
	uint32_t len = 0;

	// header
	memcpy(&out_pack_buf[len], &HEADER, 4);
	len += 4;

	// ch
	out_pack_buf[len] = m->ch;
	len += 1;

	// pack 
	// len: head len + ch len + package len + cmd len + param len + checksum len
	uint32_t pack_len = 4 + 1 + 4 + 1 + m->param_len + 1;
	memcpy(&out_pack_buf[len], &pack_len, 4);
	len += 4;

	// cmd
	out_pack_buf[len] = m->cmd;
	len += 1;

	// param
	memcpy(&out_pack_buf[len], m->param, m->param_len);
	len += m->param_len;

	// checksum
	uint8_t checksum = 0;
	for(uint32_t i = 0; i < len; i++) {
		checksum += out_pack_buf[i];
	}
	out_pack_buf[len] = checksum;
	len += 1;

	*out_pack_len = len;
}

/**
 * set a channel target value
 *  @param update_ch:				- which channel
 *  @param v:								- channel value
 *  @return:               	- none
 */
void wf_scope_sender::set_ch_target(uint8_t ch, int v) {
	if(ch > CH_5)
		return;

	_ch_target_v[ch-1] = v;
}

/**
 * set a channel current value
 *  @param update_ch:				- which channel
 *  @param v:								- channel value
 *  @return:               	- none
 */
void wf_scope_sender::set_ch_current(uint8_t ch, int v) {
	if(ch > CH_5)
		return;

	_ch_current_v[ch-1] = v;
}

	/**
	 * sync all channel value to host
	 *  @param out_len:					- package lenght
	 *  @param buf:							- output buffer
	 *  @return:               	- none
	 */
void wf_scope_sender::sync(uint32_t *out_len, uint8_t *buf) {
	uint32_t len;
	wf_scope_sender::msg m;

	*out_len = 0;
	for(uint8_t i = CH_1; i < CH_MAX; i++) {
		m.ch = i;
		m.cmd = CMD_SET_HOST_TARGET;
		m.param_len = 4;
		memcpy(m.param, &_ch_target_v[ch-1], sizeof(int));
		msg_to_pack(&m, &len, buf+(*out_len));
		*out_len += len;

		m.ch = ch;
		m.cmd = CMD_SET_HOST_CURRENT;
		m.param_len = 4;
		memcpy(m.param, &_ch_current_v[ch-1], sizeof(int));
		
		msg_to_pack(&m, &len, buf+(*out_len));
		*out_len += len;
	}
}
