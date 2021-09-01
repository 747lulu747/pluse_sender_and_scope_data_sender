/**********************************************************************/
// include header files
/**********************************************************************/
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include "console.h"


/**********************************************************************/
// extern variable declaretion
/**********************************************************************/


/**********************************************************************/
// local variable definition
/**********************************************************************/
const uint32_t console::BUF_SIZE;
const uint32_t console::RX_LINE_SIEZE;
constexpr char console::cmd_delim;
constexpr uint32_t register_command::CMD_CNT;


/**********************************************************************/
// function definition
/**********************************************************************/
void console::start(void) {
  xTaskCreate(
    main_loop				// task function
    ,  "con_main"  	// A name just for humans
    ,  1024     		// This stack size can be checked & adjusted by reading the Stack Highwater
    ,  this     		// Parameter
    ,  2        		// Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
}

void console::main_loop(void *param) {
	bool new_line;
	class console *con = (class console *)param;
	
	for(;;) {
		if(!con->_port->available()) {
			vTaskDelay( configTICK_RATE_HZ/100 );
			continue;
		}

		while(Serial.available()){
			if(con->rx_len < (console::RX_LINE_SIEZE-1)){
				con->rx_line[con->rx_len++] = con->_port->read();
			}
			else{
				con->rx_len = 0;
			}
			
			if(con->rx_len < 2)
				continue;

			if(con->rx_line[con->rx_len-2] == '\r' && con->rx_line[con->rx_len-1] == '\n') {
				con->rx_line[con->rx_len-2] = '\0';
				new_line = true;
			}
			if(con->rx_line[con->rx_len-1] == '\n') {
				con->rx_line[con->rx_len-1] = '\0';
				new_line = true;
			}
			if(new_line) {
				/**
				 * make a string with end-char 
				 */
				// con->rx_line[con->rx_len-2] = '\0';
				char *p, *q;
				p = q = (char *)con->rx_line;
				do {
					if(*q != console::cmd_delim) {
						q++;
					}
					else {
						/**
						 * get a string segment, make string end of char \0
						 * and process it
						 */
						*q ='\0';
						con->execute(p);

						/**
						 * skip all the delim 
						 */
						while(*(++q) == console::cmd_delim);
						/**
						 * p point to a new string
						 */
						p = q;
					}
				} while(*q);
				/**
				 * the last string segment
				 */
				con->execute(p);

				/**
				 * reset this len 
				 */
				con->rx_len = 0;
			}
			new_line = false;
		} // while
	} // for
}

/**
 * string command execute
 *  @param :								- none
 *  @return:               	- none
 */
void console::execute(const char *cmd) {
	_port->println(cmd);
	cmd_cb_t f = reg_cmd.find_cb(cmd[0]);
	if(f) f((char *)cmd+1);
}

/**
 * set a channel value
 *  @param :								- none
 *  @return:               	- none
 */
void console::set_channel_value(uint8_t ch, float v) {
	if(ch < simpleFOC_scope_sender::CH_MAX) {
		wf_scope.set_ch_current(ch, v);
	}

	if(ch < wf_scope_sender::CH_MAX) {
		simpleFOC_scope.set_channel_vale(ch, v);
	}
}

/**
 * send channel to host
 *  @param :								- none
 *  @return:               	- none
 */
void console::sync(uint8_t ch_maskbit) {
	uint32_t len;
	static uint8_t buf[BUF_SIZE];

	if(scope_flag & WF_SCOPE_FLAG) {
		wf_scope.sync(&len, buf);
		_port->write(buf, len);
	}

	if(scope_flag & SIMPLE_SCOPE_FLAG) {
		simpleFOC_scope.sync(&len, buf, ch_maskbit);
		_port->write(buf, len);
	}
}

