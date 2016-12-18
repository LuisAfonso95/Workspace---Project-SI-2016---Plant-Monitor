/*
 * File:   analog_read.c
 * Author: LuisRA
 *
 * 
 * inputs:
 *  - pin 25 - A10
 * 
 * Created on December 15, 2016, 7:26 PM
 */


#include "xc.h"
#include "config.h"
#include <libpic30.h>
#include <stdio.h>

#include "UART_utils.h"


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

unsigned int readADC(unsigned int ch)
{
    AD1CHS = ch;            // Select analog input channel
    AD1CON1bits.SAMP = 1;   // start sampling, then go to conversion

    while (!AD1CON1bits.DONE); // conversion done?
    return(ADC1BUF0);       // yes then get ADC value
}

inline void ConfigIO(void)
{
    ANSB = ~0x0084;         // RX1, TX1 to not analog
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB7 = 0;

    TRISBbits.TRISB14 = 1; 
}

unsigned int values[1];
int main(void) {
    
    
    ConfigCLK();
    UART1Init(9600);
    
    ConfigIO();
    ConfigADC();
    
    ANSBbits.ANSB13 = 0;
    TRISBbits.TRISB13 = 0;
    
	while(1){
        
        PORTBbits.RB13 = 1;
        __delay_ms(1); 
        values[0] = readADC(10);    
        PORTBbits.RB13 = 0;
        send_16bit_values(values, 1);
        __delay_ms(60000); 
	}
    
    
    return 0;
}
