/*
 * 
 * 
 * Measurements period: 60s
 * Timer tick period: 1ms
 * 
 * After measurement it sends data right away
 * No handshake messages - just send and hope it's received
 * 
 *  ==================================
 *  Pins:
 *  - Pin 17 - SCL1 - RB8
 *  - Pin 18 - SDA1 - RB9
 * 
 * Inputs: 
 *  - Pin 26 - Analog 9 - RB15 - soil temperature
 *  - Pin 25 - Analog 10 - RB14 - soil moisture
 *  - Pin 12 - Analog 16 - RA4 - Light meter 
 * 
 * Outputs:
 *  - Pin 19 - RA7 - Multiplexer select bits for Light meter
 * 
 * ==========================================================
 */


#include <xc.h>
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <libpic30.h>
#include <stdint.h>

#include "i2c_functions.h"
#include "SHT21.h"
#include "UART_utils.h"

/*=========== Soil Temperature Sensor Defines ===========*/
#define _BETHA_ 3435
#define _R0_ 10000
#define _T0_ 298.15
#define RDIVIDER 14950

/*=========== Timer and "scheduler" Defines ===========*/
uint32_t millis = 0;
#define MEASUREMENTS_PERIOD 60000
uint8_t Make_measurements_F = 0;

/*=========== Luminosity Sensor Defines ===========*/
#define LUX_B 13.33
#define FEEDBACK_RESISTOR1 184600
#define FEEDBACK_RESISTOR2 672
#define OFFSET_REFERENCE 36
#define MAX_READING 3849
uint32_t Rf = FEEDBACK_RESISTOR1;

/*
 * Timer init
 * 
 * Configs timer and interrupt
 * 
 */
void Timer_Init(){
    IPC0bits.T1IP = 1;

	T1CON = 0x00;
	TMR1 = 0x00;
	PR1 = 16000; // with no prescaler and 16Mhz clock = 1ms period
	T1CON =  0x8000; //starts timer,  prescaler 1:1, Internal clock ,

	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	T1CONbits.TON = 1;
    
}
// Delays a number of periods (should be in 1ms period)
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
uint16_t Get_ADC_Average(uint16_t ch, uint16_t samples){
    uint32_t values = 0; 
    int k;
    for(k = 0; k < samples; k++){
        values +=  readADC(ch);
    }
    values = values / samples;   
    return (uint16_t)values;
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
    
    //Light meter pins:
    ANSAbits.ANSA4 = 1;   
    TRISAbits.TRISA4 = 1;  
    TRISAbits.TRISA7 = 0; 
}

/*============================ Soil functions ============================*/
inline float CalculoTemperatura(unsigned int Reading){
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
    Delay(10);
    //__delay_us(10);
    
    uint16_t Vadc = Get_ADC_Average(9, 16);

   return CalculoTemperatura(Vadc) - 273.15;
}

uint16_t Get_Soil_Moisture(){
   
    uint16_t Vadc = 0;
    
    PORTBbits.RB13 = 1;
    __delay_ms(1);
    
    Vadc = Get_ADC_Average(10, 16);
    PORTBbits.RB13 = 0;
  
    
    return Vadc;
}

/*============================ Air functions ============================*/
void Get_SHT21(uint16_t *temperature, uint16_t *r_humidity){
    *r_humidity = SHT21_Read_Humidity();
    __delay_ms(1000);
    *temperature = SHT21_Read_Temperature();
    *r_humidity = SHT21_Convert_R_Humidity_10milli(*r_humidity);
    *temperature = SHT21_Convert_Temperature_10milli(*temperature);
    
    return;
}

/*============================ Light functions ============================*/
uint16_t reading_To_lux(uint16_t ADC_reading, uint32_t Offset, uint32_t RF){
    
    if(ADC_reading < Offset)
        ADC_reading = 0;
    else
        ADC_reading = ADC_reading - Offset;
    //to current:
    float uvoltage = (uint32_t)5000000/(uint32_t)4095*(uint32_t)(ADC_reading);
    float ucurrent = (uvoltage) / RF;
    float lux = LUX_B * ucurrent;
    
    uint16_t value = lux;
    return value;
}

uint16_t Get_Fotodiode_Lux(){
    Rf = FEEDBACK_RESISTOR1;
    PORTAbits.RA7 = 0;
    __delay_ms(20); 
    uint16_t temp = Get_ADC_Average(16, 16);
    if(temp > MAX_READING){
        PORTAbits.RA7 = 1;
        Rf = FEEDBACK_RESISTOR2;
        __delay_ms(20); 
        temp = Get_ADC_Average(16, 16);
    }

    temp = reading_To_lux(temp, OFFSET_REFERENCE, Rf);
    return temp;
}


/*============================ Main ============================*/
int main(void) {
    
    ConfigCLK();
    UART1Init(9600);
    ConfigIO();
    ConfigADC();
    
            
    I2CInit();
    __delay_ms(100);
    SHT21_init();
    
    Timer_Init();
    Make_measurements_F = 1;
    uint16_t Moist, Air_Temperature, Air_R_Humidity, Lux;
    float Temp;
    
    while(1){
        if(Make_measurements_F == 1){
            Temp = Get_Soil_Temperature();
            Moist = Get_Soil_Moisture();
            Get_SHT21(&Air_Temperature, &Air_R_Humidity);
            
            Lux = Get_Fotodiode_Lux();
            
            Make_measurements_F = 0;
            
            uint16_t send[5];
            send[0] = Temp * 100.0;
            send[1] = Moist;
            send[2] = Air_Temperature;
            send[3] = Air_R_Humidity;
            send[4] = Lux;
            send_16bit_values(send, 5);
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