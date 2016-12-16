/*
 * File:   NTC.c
 * Author: filip
 *
 * 
 * Inputs: 
 *  - Pin 26 - Analog 9
 * 
 * Created on December 14, 2016, 3:34 PM
 */


#include <xc.h>
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <libpic30.h>


#define SYNC1 0XAB
#define SYNC2 0X3C

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

int putChar1( int c)
{
    while ( U1STAbits.UTXBF); // wait while Tx buffer full
    U1TXREG = c;
    return c;
}// putChar1

char getChar1( void)
{
    while ( !U1STAbits.URXDA); // wait
    return U1RXREG; // read from the receive buffer
}// getChar1

int check_sum_values(unsigned int check_sum, unsigned int value){
    check_sum = check_sum + value;
    return check_sum;
}


inline void ConfigADC(void)
{
/*
 * Configura o ADC para funcionar comandado por SW
 * Consultar: Section 51 - 12-Bit AD Converter with Threshold Detect
 * e Ch 19 de PIC24FV16KM204 FAMILY Data Sheet
 */

    AD1CON1 = 0x0470;   // 12-bit A/D operation (MODE12=1)
                // SSRC<3:0> = 0111 Internal counter ends sampling
                // and starts conversion (auto-convert)
    AD1CON2 = 0x0000;   // Configure A/D voltage reference and buffer fill modes.
                // Vr+ and Vr- from AVdd and AVss(PVCFG<1:0>=00, NVCFG=0),
                // Inputs are not scanned,
                // Buffer in FIFO mode (BUFM=0)
    AD1CON3 = 0x1003;   // Sample time = 16Tad, Tad = 4Tcy (= 250ns)

    AD1CSSL = 0;        // No inputs are scanned.

    _AD1IF = 0; // Clear A/D conversion interrupt.
    _AD1IE = 0; // Disable A/D conversion interrupt
    AD1CON1bits.ADON = 1; // Turn on A/D
}

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

unsigned int readADC(unsigned int ch)
{
    AD1CHS = ch;            // Select analog input channel
    AD1CON1bits.SAMP = 1;   // start sampling, then go to conversion

    while (!AD1CON1bits.DONE); // conversion done?
    return(ADC1BUF0);       // yes then get ADC value
}

inline void ConfigIO(void)
{
    ANSB = ~0x0484;         // RX1, TX1, RB10 digitais, restantes analogicos;
    TRISB= 0b1111111101111111;
}
inline float CalculoTemperatura(float Vadc){
    
    float R1,RT,R0,Vout,V,beta,T0,T,Temp;
    T0=298.15;
    V = 5;
    beta=2500;
    R1=15060;
    R0=10000;
    Vout=(5*Vadc)/4095;
    RT=(R1*Vout)/(V-Vout);
    T=(log(RT/R0)/beta)+1/T0;
    T=1/T;
    Temp=T-273.15;
    return Temp;
}

int main(void) {
    
    ConfigCLK();
    UART1Init(19200);
    ConfigIO();
    ConfigADC();
    int i;
    float Vadc,Temp;
    while(1){
        Vadc=0;
        for(i = 0; i < 16; i++){
            Vadc += readADC(9);     // Adquire canal 9
            __delay_ms(6); 
        }
        Vadc=Vadc/16;
        Temp = CalculoTemperatura(Vadc);
        printf("\r%f",Temp);
    }
    
    
    return 0;
}
