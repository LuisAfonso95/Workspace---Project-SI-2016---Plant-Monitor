/* 
 * 
 * pins:
 *  SDA pin 18 (RB9)
 *  SCL pin 17 (RB8)
 * 
 * 
 */

#include "xc.h"
#include "config.h"
#include <libpic30.h>
#include <stdio.h>
#include <stdint.h>
#include "i2c_functions.h"
#include "SHT21.h"

/*==========================================================
    UART functions
 */

inline void ConfigCLK(void)
{
    //configurar oscilador para 32MHz
    CLKDIVbits.DOZE = 0;    // 1:1
    CLKDIVbits.RCDIV = 0;   // 8 MHz
}

inline void UART1Init(unsigned long int baud)
{
    U1BRG = (FCY / (16 * baud)) -1; // 19200 bps @ 16 MHz -> BRG = 51
    U1MODE= 0x8000; // UARTEN
    U1STA = 0x0400; // enable transmission
}// UART1Init

// wait for a new character to arrive to the UART1 serial port
char getChar1( void)
{
    while ( !U1STAbits.URXDA); // wait
    return U1RXREG; // read from the receive buffer
}// getChar1

int putChar1( int c)
{
    while ( U1STAbits.UTXBF); // wait while Tx buffer full
    U1TXREG = c;
    return c;
}// putChar1

int check_sum_values(unsigned int check_sum, unsigned int value){
    check_sum = check_sum + value;
    return check_sum;
}

/*
 
 how it works, each | | is a section, usually just 1 byte (all in the same line)
 
 | sync1 (1byte) | sync2 (1byte) | command (1byte) | number of packets (1byte) | 

 | packets (0 to 255 bytes) | check_sum (1byte)|
 
 
 */
#define SYNC1 0XAB
#define SYNC2 0X3C
int send_16bit_values(unsigned int values[], int size){
  
    int check_sum = 0;
    
    //check_sum +=number_of_ones(SYNC1);
    check_sum_values(check_sum, SYNC1);
    putChar1(SYNC1);
    //check_sum +=number_of_ones(SYNC2);  
    check_sum_values(check_sum, SYNC2);
    putChar1(SYNC2);
    
    //command 0x12 (18) send 16bit values to MatLab
    check_sum_values(check_sum, 0x12);
    putChar1(0x12);
  
    //packet size (multiples of 2)
    check_sum_values(check_sum, size*2);
    putChar1(size*2);
    
    int i;
    for(i = 0; i < size; i++){
        //check_sum +=number_of_ones(c & 0xFF);
        check_sum_values(check_sum, values[i] & 0xFF);
        putChar1(values[i] & 0xFF);
        //check_sum +=number_of_ones(c >> 8);
        check_sum_values(check_sum, values[i] >> 8);
        putChar1(values[i] >> 8); 
    }

    putChar1(check_sum);
    
  return 0;
}






unsigned int values[2];
int main(void) {
    
    ANSB = 0;
    ConfigCLK();
    UART1Init(9600);
    
    I2CInit();
    __delay_ms(100);
    SHT21_init();
    
    unsigned int last_values[2] = {0, 0};
    while(1){

        //char reg = SHT21_Read_user_register();
        values[0] = SHT21_Read_Humidity();
        __delay_ms(1000);
       values[1] = SHT21_Read_Temperature();
  
  
       if(values[0] == 65535)
           values[0] = last_values[0];
       else
           last_values[0] = values[0];
       if(values[1] == 65535)
           values[1] = last_values[1];
       else
           last_values[1] = values[1];
        send_16bit_values(values, 2);
        __delay_ms(1000);
    }
    
}



