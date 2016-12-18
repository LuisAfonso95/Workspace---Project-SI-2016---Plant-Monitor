/*
 * 
 * 
 * NTC luis:
 *  (R1 = 14.95kohm)
 *   At 100ºC (283.15K) 883.17ohm (883ohm)
 *  At 0ºC (271.15K), 26.139kohm (check resistor 1 value)
 * 
 * B = 3469 ohm
 * 
 * Inputs: 
 *  - Pin 26 - Analog 9 - soil temperature
 *  - Pin 25 - Analog 10 - soil moisture
 * 
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

uint32_t millis = 0;
#define MEASUREMENTS_PERIOD 60000
uint8_t Send_stuff_F = 0;
uint8_t Make_measurements_F = 0;

void Timer_Init(){
    IPC0bits.T1IP = 1;

	T1CON = 0x00;
	TMR1 = 0x00;
	PR1 = 16000;
	T1CON =  0x8000; //starts timer,  prescaler 1:1, Internal clock ,

	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	T1CONbits.TON = 1;
    
}
void Delay(uint32_t _ms){
	volatile uint32_t _temp = millis;
	volatile uint32_t _deltaT = millis - _temp;
	while(_deltaT < _ms){
		_deltaT = millis - _temp;
	}
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
    
    TRISBbits.TRISB15 = 1; //temperature sensor
             
    TRISBbits.TRISB14 = 1; //moisture sensor input
    
    //Moisture sensor power pin
    ANSBbits.ANSB13 = 0;
    TRISBbits.TRISB13 = 0;
    PORTBbits.RB13 = 0; // start with power off
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
    float T = (log(RT / R0) / beta) + (1.0 / T0);
    T = 1.0 / T;
    //T = T-273.15;
    
    return T;
}
float Get_Soil_Temperature(){
    AD1CHS = 9;   
    Delay(1);
    //__delay_us(10);
    
    uint16_t Vadc = 0;
    int i;
    for(i = 0; i < 16; i++){
        //AD1CON1bits.SAMP = 1;   // start sampling, then go to conversion
        //while (!AD1CON1bits.DONE); // conversion done?
        
        Vadc += readADC(9);//ADC1BUF0;     // Adquire canal 9
    }
    Vadc=Vadc/16;
   return CalculoTemperatura(Vadc);
}

uint16_t Get_Soil_Moisture(){
   
    uint16_t Vadc = 0;
    
    PORTBbits.RB13 = 1;
    __delay_ms(1);
    
    int i;
    for(i = 0; i < 16; i++){
        Vadc += readADC(10);     // Adquire canal 9
    }
    PORTBbits.RB13 = 0;
    Vadc=Vadc/16; 
    
    return Vadc;
}

int main(void) {
    
    ConfigCLK();
    UART1Init(9600);
    ConfigIO();
    ConfigADC();
    
    Timer_Init();
    
    uint16_t Moist;
    float Temp;
    while(1){

        if(Make_measurements_F == 1){
            Temp = Get_Soil_Temperature();
            Moist = Get_Soil_Moisture();
            Make_measurements_F = 0;
            Send_stuff_F = 1;
        }
        else if(Send_stuff_F == 1){
            uint16_t send[2];
            send[0] = Temp * 100.0;
            send[1] = Moist;
            send_16bit_values(send, 2);
            Send_stuff_F = 0;
        }
        else{
            //sleep
        }

    }
    
    
    return 0;
}



void _ISR _T1Interrupt (void)
{
	/* Interrupt Service Routine code goes here */
	_T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
	
	if(millis == 0xFFFFFFFF)
		millis = 0;
	else
		millis++;
	
    if(millis % MEASUREMENTS_PERIOD == 0){
        Make_measurements_F = 1;
    }
    
            
    /*if(millis % SOIL_MOISTURE_PERIOD == 0){
        Soil_Moisture_F = 1;
    }
    if(millis % SOIL_TEMPERATURE_PERIOD == 0){
       Soil_Temperature_F = 1;
    }
    //if(millis % SEND_STUFF_PERIOD == 0){
    //   Send_stuff_F = 1;
    //}	*/
    
}