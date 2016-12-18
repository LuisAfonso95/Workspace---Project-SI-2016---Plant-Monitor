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
#include "UART_utils.h"

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



