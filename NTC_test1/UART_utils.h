/* 
 * File:   UART_utils.h
 * Author: LuisRA
 *
 * Created on December 17, 2016, 8:02 PM
 */

#ifndef UART_UTILS_H
#define	UART_UTILS_H

#include "xc.h"
//#include "stdint.h"

inline void ConfigCLK(void);
inline void UART1Init(unsigned long int baud);
int putChar1( int c);
char getChar1( void);

int check_sum_values(unsigned int check_sum, unsigned int value);
int send_16bit_values(unsigned int values[], int size);


#endif	/* UART_UTILS_H */

