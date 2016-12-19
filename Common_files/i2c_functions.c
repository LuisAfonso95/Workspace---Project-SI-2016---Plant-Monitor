
#include "i2c_functions.h"
/*
Function: I2CInit
Return:
Arguments:
Description: Initialize I2C in master mode, Sets the required baudrate
*/
void I2CInit(void)
{
    ANSBbits.ANSB8 = 0;
    ANSBbits.ANSB9 = 0;
	TRISBbits.TRISB9 = 1;      /* SDA and SCL as input pin */
	TRISBbits.TRISB8 = 1;      /* these pins can be configured either i/p or o/p */
	SSP1STAT = 0b11000000; /* Slew rate disabled */
	SSP1CON1 = 0b00101000;   /* SSPEN = 1, I2C Master mode, clock = FOSC/(4 * (SSPADD + 1)) */
    SSP1CON2 = 0;
    //SSP1CON3bits.SDAHT = 1;
	SSP1ADD = 159;    /* 100Khz @ 16Mhz Fosc */
}
 
/*
Function: I2CStart
Return:
Arguments:
Description: Send a start condition on I2C Bus
*/
void I2CStart()
{

	//while(SSP1CON2bits.SEN );      /* automatically cleared by hardware */
    I2CWait();
    SSP1CON2bits.SEN = 1;         /* Start condition enabled */
                     /* wait for start condition to finish */
}
 
/*
Function: I2CStop
Return:
Arguments:
Description: Send a stop condition on I2C Bus
*/
void I2CStop()
{
	SSP1CON2bits.PEN = 1;         /* Stop condition enabled */
	while(SSP1CON2bits.PEN);      /* Wait for stop condition to finish */
                     /* PEN automatically cleared by hardware */
}
 
/*
Function: I2CRestart
Return:
Arguments:
Description: Sends a repeated start condition on I2C Bus
*/
void I2CRestart()
{
	SSP1CON2bits.RSEN = 1;        /* Repeated start enabled */
	while(SSP1CON2bits.RSEN);     /* wait for condition to finish */
}
 
/*
Function: I2CAck
Return:
Arguments:
Description: Generates acknowledge for a transfer
*/
void I2CAck()
{
	SSP1CON2bits.ACKDT = 0;       /* Acknowledge data bit, 0 = ACK */
	SSP1CON2bits.ACKEN = 1;       /* Ack data enabled */
	while(SSP1CON2bits.ACKEN);    /* wait for ack data to send on bus */
}
 
/*
Function: I2CNck
Return:
Arguments:
Description: Generates Not-acknowledge for a transfer
*/
void I2CNak()
{
	SSP1CON2bits.ACKDT = 1;       /* Acknowledge data bit, 1 = NAK */
	SSP1CON2bits.ACKEN = 1;       /* Ack data enabled */
	while(SSP1CON2bits.ACKEN);    /* wait for ack data to send on bus */
}
 
/*
Function: I2CWait
Return:
Arguments:
Description: wait for transfer to finish
*/
void I2CWait()
{
	while ((SSP1CON2 &  0x1F ) || ( SSP1STAT & 0x04 ) );
    /* wait for any pending transfer */
}
 
/*
Function: I2CSend
Return:
Arguments: dat - 8-bit data to be sent on bus
           data can be either address/data byte
Description: Send 8-bit data on I2C bus
*/
void I2CSend(unsigned char dat)
{
    I2CWait();
	SSP1BUF = dat;    /* Move data to SSPBUF */
	//while(SSP1STATbits.BF);       /* wait till complete data is sent from buffer */
    while(SSP1STAT & 0x4);
	//I2CWait();       /* wait for any pending transfer */
}
 
/*
Function: I2CRead
Return:    8-bit data read from I2C bus
Arguments:
Description: read 8-bit data from I2C bus
*/
#define FOSC	32000000UL
#define FCY	(FOSC/2)
#include <libpic30.h>
uint8_t I2CRead(void)
{
	unsigned char temp;
/* Reception works if transfer is initiated in read mode */
	SSP1CON2bits.RCEN = 1;        /* Enable data reception */
	while(!SSP1STATbits.BF ){
    }      /* wait for buffer full */
	temp = SSP1BUF;   /* Read serial buffer and store in temp register */
	return temp;     /* Return the read data from bus */
}


uint8_t I2C_Timed_Read(uint32_t timeout){
    SSP1CON2bits.RCEN = 1; 
    volatile uint16_t k =0;
    while(!SSP1STATbits.BF && k < timeout){
        k++;
        __delay_us(1);
    } 
    if(k >= timeout){
        return -1;
    }
    else{
        return SSP1BUF;
    }
}
void I2C_Start_Read(){
    SSP1CON2bits.RCEN = 1; 
    
    return;
}

int16_t I2C_Get_Read(){
    if(SSP1STATbits.BF == 1){
        return -1;
    }
    else{
        return SSP1BUF;
    }
    
}



int I2C_Write(char addr, char value){
    I2CStart();
    addr = (addr << 1);
    I2CSend(addr);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    }
    
    I2CSend(value);
    
    if(SSP1CON2bits.ACKSTAT == 1){
        return -1;
    } 
    
    return 0;
}

