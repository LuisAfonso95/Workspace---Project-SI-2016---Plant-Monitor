/*
 * File:   NTC.c
 * Author: filip
 *
 * 
 * At 100ºC (283.15K), read 252 = 0.3076V = 0.99383kohm
 * At 0ºC (271.15K)(not accurate), 25kohm (parece ser acima disso)
 * B = 3.287kohm
 * 
 * NTC luis:
 *  (14.95kohm)
 *   At 100ºC (283.15K) 883.17ohm (883ohm)
 *  At 0ºC (271.15K), 26.139kohm (check resistor 1 value)
 * 
 * B = 3469 ohm
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
#include <stdint.h>

#include "UART_utils.h"
#define _BETHA_ 3469
#define _R0_ 26139
#define _T0_ 273.15
#define RDIVIDER 14950


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
    ANSB = ~0x0484;         // RX1, TX1, RB10 digitais, restantes analogicos;
    TRISB= 0b1111111101111111;
}


inline float CalculoTemperatura(unsigned int Reading){
    
    //float R1,RT,R0,Vout,V,beta,T0,T,Temp;
    float T0 = _T0_;
    float V = 5;
    float beta = _BETHA_;
    float R1 = RDIVIDER;
    float R0 = _R0_;
    float Vout =( 5.0 * Reading) / 4096.0;
    float RT = ((float)R1 * Vout) / ((float)V - Vout);
    float T = (log(RT / R0) / beta) + (1 / T0);
    T= 1 / T;
    //T = T-273.15;
    
    return T;
}

int main(void) {
    
    ConfigCLK();
    UART1Init(9600);
    ConfigIO();
    ConfigADC();
    int i;
    unsigned int Vadc;
    float Temp;
    while(1){
        Vadc=0;
        for(i = 0; i < 16; i++){
            Vadc += readADC(9);     // Adquire canal 9
            __delay_ms(6); 
        }
        Vadc=Vadc/16;
        Temp = (CalculoTemperatura(Vadc));
        uint16_t send = Temp * 100.0;
        send_16bit_values(&send, 1);
        //(&Temp, 1);
        //printf("%f\r\n",Temp);
    }
    
    
    return 0;
}
