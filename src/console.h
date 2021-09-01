#pragma once
/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "wf_scope_sender.h"
#include "simpleFOC_scope_sender.h"


/**********************************************************************/
// macro define
/**********************************************************************/


/**********************************************************************/
// type define
/**********************************************************************/
typedef void (*cmd_cb_t)(const char *);

class register_command{

public:
	/**
	 * the number of command that can been registered
	 */
	static constexpr uint32_t CMD_CNT = 16;

	register_command() {
		for(uint32_t i = 0; i < CMD_CNT; i++) {
			_c[i] = 0;
			_cb[i] = nullptr;
		}
	}

	/**
	 * find a call back function with the first letter 
	 *  @param letter:					- first letter of the register command
	 *  @return:               	- index of this register command, -1 means NOT found
	 */
	int find(char letter) {
		for(uint32_t i = 0; i < CMD_CNT; i++) {
			if(_c[i] == letter)
				return i;
		}
		return -1;
	}

	/**
	 * find a call back function with the first letter 
	 *  @param letter:					- first letter of the register command
	 *  @return:               	- pointer or nullptr
	 */
	cmd_cb_t find_cb(char letter) {
		for(uint32_t i = 0; i < CMD_CNT; i++) {
			if(_c[i] == letter)
				return _cb[i];
		}
		return nullptr;
	}

	/**
	 * insert a call function to the first available index
	 * @param letter:						- first letter of the register command
	 * @param cb:								- the call back function pointer
	 * @return:               	- index of this register command, -1 means insert failed
	 */
	int insert(char letter, cmd_cb_t cb) {
		for(uint32_t i = 0; i < CMD_CNT; i++) {
			if(_c[i] == 0){
				_c[i] = letter;
				_cb[i] = cb;
				return i;
			}
		}
		return -1;
	}

	/**
	 * register a command callback
	 *  @param letter:					- first letter of the register command
	 *  @param f                - the call back function pointer
	 *  @return:               	- true or false
	 */
	bool register_cmd(char letter, void(*f)(const char *)) {
		int index = find(letter);
		if(-1 == index){
			return insert(letter, f) != -1 ? true : false;
		}
		else{
			_c[index] = letter;
			_cb[index] = f;
			return true;
		}
	}

private:
	char _c[CMD_CNT];					// <! the first letter of the register command
	cmd_cb_t _cb[CMD_CNT];		// <! the call back function pointer
};

class console
{
public:
	static const uint32_t BUF_SIZE = 512;
	static const uint32_t RX_LINE_SIEZE = 128;

	enum {
		CH_1 = 0,
		CH_2,
		CH_3,
		CH_4,
		CH_5,
		CH_6,
		CH_7,
		CH_MAX
	};

	enum {
		WF_SCOPE_FLAG = 1<<0,
		SIMPLE_SCOPE_FLAG = 1<<1,
	};

	/**
	 * 
	 */
	static constexpr char cmd_delim = ' ';

	/**
	 * 
	 */
	class wf_scope_sender wf_scope;

	/**
	 * 
	 */
	class simpleFOC_scope_sender simpleFOC_scope;

	/**
	 * 
	 */
	console(Stream &port) : _port(&port), rx_len(0), ch_out_maskbit(0xFF) {}

	/**
	 * start a task, read data from stream port and execute it
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void start(void);

	/**
	 * set a channel value
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void set_channel_value(uint8_t ch, float v);

	/**
	 * send channel to host
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void sync(uint8_t ch_maskbit);

	/**
	 * set wf_scope or simpleFOC scope flag
	 * which will detemind whether to send when synchronize channel
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void set_scope_agency_flag(uint8_t flag) { scope_flag = flag; }

	/**
	 * register a command callback
	 *  @param letter:					- first letter of the register command
	 *  @param f                - the call back function pointer
	 *  @return:               	- true or false
	 */
	bool register_cmd(char letter, void(*f)(const char *)) { return reg_cmd.register_cmd(letter, f); }

private:
	Stream *_port;
	uint32_t rx_len;
	uint8_t rx_line[RX_LINE_SIEZE];
	uint8_t scope_flag;
	register_command reg_cmd;
	uint8_t ch_out_maskbit;

	/**
	 * main loop for console, must been static
	 *  @param :								- none
	 *  @return:               	- none
	 */
	static void main_loop(void *param);

	/**
	 * string command execute
	 *  @param :								- none
	 *  @return:               	- none
	 */
	void execute(const char *cmd);
}; 


/**********************************************************************/
// extern variable
/**********************************************************************/


/**********************************************************************/
// extern function declaretion
/**********************************************************************/

