/*
 * File:   analog_read.c
 * Author: LuisRA
 *
 * Created on December 15, 2016, 7:26 PM
 */


#include "xc.h"
#include "config.h"
#include <libpic30.h>
#include <stdio.h>
#include <stdint.h>

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
    
    ANSAbits.ANSA4 = 1;   
    TRISAbits.TRISA4 = 1; 
    
    //ANSAbits. = 0;   
    TRISAbits.TRISA7 = 0; 
}

#define LUX_B 13.33
#define FEEDBACK_RESISTOR1 184600
#define FEEDBACK_RESISTOR2 672
#define OFFSET_REFERENCE 750000
#define MAX_MVOLTAGE 3600
#define MAX_READING 3480
#define MIN_READING 630
uint16_t reading_To_lux(uint16_t ADC_reading, uint32_t uV_Offset, uint32_t RF){
    
    
    //to current:
    float uvoltage = (uint32_t)5000000/(uint32_t)4096*(uint32_t)ADC_reading;
    float ucurrent = (uvoltage-uV_Offset) / RF;
    float lux = LUX_B * ucurrent;
    
    uint32_t value = lux;
    return value;
}
unsigned int values[1];
int main(void) {
    
    
    ConfigCLK();
    UART1Init(9600);
    
    ConfigIO();
    ConfigADC();
    
    PORTAbits.RA7 = 0;
   
    uint32_t Rf = FEEDBACK_RESISTOR1;
	while(1){
      

        values[0] = (uint16_t)reading_To_lux(temp, OFFSET_REFERENCE, Rf);

        send_16bit_values(values, 1);
        __delay_ms(100); 
	}
    
    
    return 0;
}
