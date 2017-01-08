/*
 *
 * B, T0 and R0 are taken from the datasheet
 * R divider = 14.95K
 * 
 * Inputs: 
 *  - Pin 26 - Analog 9
 * 
 */


#include <xc.h>
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <libpic30.h>
#include <stdint.h>

#include "UART_utils.h"
#define _BETHA_ 3435
#define _R0_ 10000 //at 25ºC
#define _T0_ 298.15
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

uint16_t Get_ADC_Average(uint16_t ch, uint16_t samples){
    uint32_t values  = 0; 
    int k;
    for(k = 0; k < samples; k++){
        values +=  readADC(ch);
    }
    values = values / samples;   
    return (uint16_t)values;
}

inline float CalculoTemperatura(unsigned int Reading){
    float T0 = _T0_;
    float V = 5;
    float beta = _BETHA_;
    float R1 = RDIVIDER;
    float R0 = _R0_;
    float Vout =( 5.0 * Reading) / 4096.0;
    float RT = ((float)R1 * Vout) / ((float)V - Vout);
    float T = (log(RT / R0) / beta) + (1 / T0);
    T= 1 / T;
    return T;
}
float Get_Soil_Temperature(){
    AD1CHS = 9;   
    //Delay(10);
    __delay_ms(10);
    
    uint16_t Vadc = Get_ADC_Average(9, 16);

   return CalculoTemperatura(Vadc) - 273.15;
}

int main(void) {
    
    ConfigCLK();
    UART1Init(9600);
    ConfigIO();
    ConfigADC();
    float Temp;
    while(1){
        Temp = Get_Soil_Temperature();
        uint16_t send = Temp * 100;
        send_16bit_values(&send, 1);
        __delay_ms(100);
    }
    return 0;
}
