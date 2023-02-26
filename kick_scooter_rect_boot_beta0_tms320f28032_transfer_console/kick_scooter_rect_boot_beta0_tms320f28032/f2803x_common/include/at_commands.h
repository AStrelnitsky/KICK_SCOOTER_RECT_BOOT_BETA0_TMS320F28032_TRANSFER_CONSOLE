/*
 * at_commands.h
 *
 *  Created on: Jan 12, 2022
 *      Author: AS
 */

#ifndef F2803X_COMMON_INCLUDE_AT_COMMANDS_H_
#define F2803X_COMMON_INCLUDE_AT_COMMANDS_H_

#include "stdint.h";
#include "stdio.h";
#include <string.h>

#define STR 1
#define NUM 0
#define LEN 2
#define STA 1
#define AP 2

#define AT(x)                   (x == 0 ? 0 : ((x) == 1 ? "AT\r\n" : 4))
#define AT_UART_Q(x)            (x == 0 ? 1 : ((x) == 1 ? "AT+UART=?\r\n" : 11))
#define AT_CWMODE_STA(x)        (x == 0 ? 2 : ((x) == 1 ? "AT+CWMODE_CUR=1\r\n" : 17))
#define AT_CIPSTART_UDP(x)      (x == 0 ? 3 : ((x) == 1 ? "AT+CIPSTART=\"UDP\",\"192.168.4.1\",7,7200\r\n" : 40))
#define AT_CIPSTART_TCP(x)      (x == 0 ? 4 : ((x) == 1 ? "AT+CIPSTART=\"TCP\",\"192.168.4.2\",7,7200\r\n" : 40))
#define AT_CWLAP_SET(x)         (x == 0 ? 5 : ((x) == 1 ? "AT+CWLAP=\"Wi-Fi\",\"06:ea:56:a0:47:81\",5,0,10,200\r\n" : 49))
#define AT_CWLAP_EXE(x)         (x == 0 ? 6 : ((x) == 1 ? "AT+CWLAP\r\n" : 10))
#define AT_CWLAPOPT(x)          (x == 0 ? 7 : ((x) == 1 ? "AT+CWLAPOPT=1,7\r\n" : 17))
#define AT_CWQAP(x)             (x == 0 ? 8 : ((x) == 1 ? "AT+CWQAP\r\n" : 10))
#define AT_CIPMODE_TR(x)        (x == 0 ? 9 :  ((x) == 1 ? "AT+CIPMODE=1\r\n" : 14))
#define AT_CIPSEND_TR(x)        (x == 0 ? 10 : ((x) == 1 ? "AT+CIPSEND\r\n" : 12))
#endif /* F2802X_COMMON_INCLUDE_AT_COMMANDS_H_ */
