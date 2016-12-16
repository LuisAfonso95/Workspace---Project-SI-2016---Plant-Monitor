
#include "SHT21.h"

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
    int reg = I2C_Timed_Read();
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
    uint8_t reg = I2C_Timed_Read();
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
            
     
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    } 
     
    I2CStop();
    
    return 0;
    
}



unsigned int SHT21_Read_Temperature(){
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
      // if( I2C_Write(64, T_MEASUREMENT) == -1)
        //   return -1;
    I2CRestart();
    send = (addr << 1) + 1;
    I2CSend(send); 
     if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }
    //while(PORTBbits.RB8 == 0);
    //__delay_us(500);
    //while(SSP1STATbits.BF);
    uint8_t data[2];
    data[0] = I2C_Timed_Read();
    I2CAck();
    data[1] = I2C_Timed_Read();
    I2CAck();
    uint8_t check_sum = I2C_Timed_Read();
    I2CNak();
    I2CStop();
    
    if(SHT2x_CheckCrc(data, 2, check_sum) == -1)
        return 65535;
        
    unsigned int value = data[0] << 8;
    value |= data[1];
    return value;
}

unsigned int SHT21_Read_Humidity(){
    uint8_t addr = 64;
    I2CStart();
    uint8_t send = (addr << 1);
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
    //while(PORTBbits.RB8 == 0);
    uint8_t data[2];
    data[0] = I2C_Timed_Read();
    I2CAck();
    data[1] = I2C_Timed_Read();
    I2CAck();
    uint8_t check_sum = I2C_Timed_Read();
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
   //__delay_ms(15); 
} 