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
#include "UART_utils.h"


const unsigned int POLYNOMIAL = 0x131; 
int SHT2x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
//==============================================================================
{
    uint8_t crc = 0;
    uint8_t byteCtr;
    //calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);

        int k;
        for (k = 8; k > 0; --k)
        { 
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
    if (crc != checksum) return -1;
    else return 0;
}


#define T_MEASUREMENT 0xE3
#define H_MEASUREMENT 0xE5
#define T_MEASUREMENT2 0xF3
#define WRITE_USER_REGISTER 0xE6
#define READ_USER_REGISTER 0xE7 
#define SOFT_RESET 0xFE
int SHT21_Read_user_register(){
    char addr = 64;
    
    //ask to read register
    I2CStart();
    uint8_t send = (addr << 1);
    I2CSend(send);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }  
    I2CSend(READ_USER_REGISTER);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }   
    
    //start reading
    I2CRestart();
    send = (addr << 1) +1;
    I2CSend(send);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }    
    int reg = I2CRead();
    I2CNak();
    I2CStop();
    return reg;
}

int SHT21_Set_user_register(){
    
    uint8_t addr = 64;
    
    //ask to read register
    I2CStart();
    uint8_t send = (addr << 1);
    I2CSend(send);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }  
    I2CSend(READ_USER_REGISTER);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }   
    
    //start reading
    I2CRestart();
    send = (addr << 1) +1;
    I2CSend(send);
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }    
    uint8_t reg = I2CRead();
    I2CNak();
    //I2CStop();
    
    reg &= ~(0b10000001);
    reg |= 0b10000001;
    reg = 0b11;
            
    I2CRestart();
    send = (addr << 1);
    I2CSend(send);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }
    
    I2CSend(WRITE_USER_REGISTER);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    } 
    
    I2CSend(reg);
    __delay_us(100);
            
     
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    } 
     
    I2CStop();
    
    return 0;
    
}



uint16_t SHT21_Read_Temperature(){
    uint8_t addr = 64;
    I2CStart();
    uint8_t send = (addr << 1);
    I2CSend(send);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }
    
    I2CSend(T_MEASUREMENT);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }

    I2CRestart();
    send = (addr << 1) + 1;
    I2CSend(send); 
     if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }

    uint8_t data[2];
    I2C_Start_Read();
    
    /*int16_t temp = -1; 
    int k = 0;
    do{
       __delay_us(1);
       k++;
       temp = I2C_Get_Read();
    }while(temp == -1 && k < 500);  
    if(k > 500){
        return -1;
    }*/
    
    data[0] = I2CRead();
    I2CAck();
    data[1] = I2CRead();
    I2CAck();
    char check_sum = I2CRead();
    I2CNak();
    I2CStop();
    
    if(SHT2x_CheckCrc(data, 2, check_sum) == -1)
        return 65535;
        
    unsigned int value = data[0] << 8;
    value |= data[1];
    return value;
}

unsigned int SHT21_Read_Humidity(){
    char addr = 64;
    I2CStart();
    char send = (addr << 1);
    I2CSend(send);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }
    
    I2CSend(H_MEASUREMENT);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }

    
    I2CRestart();
    send = (addr << 1) + 1;
    I2CSend(send); 
     if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }

    uint8_t data[2];
    data[0] = I2CRead();
    I2CAck();
    data[1] = I2CRead();
    I2CAck();
    char check_sum = I2CRead();
    I2CNak();
    I2CStop();
    
    if(SHT2x_CheckCrc(data, 2, check_sum) == -1)
        return -1;
        
    unsigned int value = data[0] << 8;
    value |= data[1];
    
    return value;
}

int SHT21_init(void){ 
   I2CStart(); 
   I2CSend(64); 
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }    
   I2CSend(SOFT_RESET); 
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }    
   I2CStop(); 
   __delay_ms(15); 
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
        /*int i = 0;
        for( i=0;  i< 256; i++){
            I2CStart();
            I2CSend(i);
            I2CWait();
            if(SSP1CON2bits.ACKSTAT == 0){
                break;
            }
            __delay_ms(1);
        }
        while(1);
    }*/
    
    return 0;
}

