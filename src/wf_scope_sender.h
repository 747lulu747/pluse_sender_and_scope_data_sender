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
/*
 * WildFire scope sender
 */
class wf_scope_sender
{
public:
	/**
	 * const define
	*/
	static const uint32_t CH_MAX = 5;
	static const uint32_t BUF_SIZE = 256;
	static const uint32_t HEADER = 0x59485a53;
	
	/**
	 * FSM state
	*/
	enum{
		FSM_HEADER_S = 0,
		FSM_ADDR_S,
		FSM_LED_S,
		FSM_DATA_S,
		FSM_CHECK_S,
	};

	/**
	 * command id define
	*/
	enum {
		CMD_SET_HOST_TARGET	= 0x01,
		CMD_SET_HOST_CURRENT,
		CMD_SET_HOST_PID,
		CMD_SET_HOST_START,
		CMD_SET_HOST_STOP,
		CMD_SET_HOST_PERIOD,

		CMD_SET_SLAVE_PID = 0x10,
		CMD_SET_SLAVE_TARGET,
		CMD_SET_SLAVE_START,
		CMD_SET_SLAVE_STOP,
		CMD_SET_SLAVE_RESET,
		CMD_SET_SLAVE_PERIED,
	};

	enum {
		CH_1 = 1,
		CH_2,
		CH_3,
		CH_4,
		CH_5,
	};

	struct msg{
		uint8_t 	ch;
		uint8_t		cmd;
		uint16_t	param_len;
		uint8_t		param[BUF_SIZE];
	};

	/**
	 * Channel value
	*/
	int _ch_target_v[CH_MAX];
	int _ch_current_v[CH_MAX];

	/*
	 *
	 */
	wf_scope_sender():state(FSM_HEADER_S){};
	~wf_scope_sender(){};

	/**
	 * put a stream char to fsm. fsm will rebuild a package from a stream data
	 *  @param c						 		- stream data: char c
	 *  @return:               	- true: a pack have been recieve sussessfully
	 * 													- flase: no pack is available
	 */
	bool putchar(char c);

	/**
	 * return a msg. This msg come from the recieve data
	 *  @param m						 		- msg poiter for output
	 *  @return:               	- none
	 */
	void get_msg(struct msg* m);

	/**
	 * build a stream package frome the message
	 *  @param m						 		- msg poiter for output
	 *  @param out_pack_len			- the output stream package lenght
	 *  @param out_pack_buf			- the output stream package buffer
	 *  @return:               	- none
	 */
	void msg_to_pack(struct msg* m, uint32_t *out_pack_len, uint8_t *out_pack_buf);

	/**
	 * set a channel target value
	 *  @param update_ch:				- which channel
	 *  @param v:								- channel value
	 *  @return:               	- none
	 */
	void set_ch_target(uint8_t ch, int v);

	/**
	 * set a channel current value
	 *  @param update_ch:				- which channel
	 *  @param v:								- channel value
	 *  @return:               	- none
	 */
	void set_ch_current(uint8_t ch, int v);

	/**
	 * sync all channel value to host
	 *  @param out_len:					- package lenght
	 *  @param buf:							- output buffer
	 *  @return:               	- none
	 */
	void sync(uint32_t *out_len, uint8_t *buf);

private:
	uint8_t state;
	uint8_t ch;
	uint8_t pack_len;
	uint8_t have_rx_len;
	uint8_t checksum;
	uint8_t data[BUF_SIZE];
};

/**********************************************************************/
// extern function declaretion
/**********************************************************************/

