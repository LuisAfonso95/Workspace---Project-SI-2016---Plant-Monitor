
#include "UART_utils.h"

/* UART part */
inline void ConfigCLK(void)
{
    //configurar oscilador para 32MHz
    CLKDIVbits.DOZE = 0;    // 1:1
    CLKDIVbits.RCDIV = 0;   // 8 MHz
}

#define FOSC	32000000UL
#define FCY	(FOSC/2)
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



/* Message packets part */
int check_sum_values(unsigned int check_sum, unsigned int value){
    check_sum = check_sum + value;
    return check_sum;
}


#define SYNC1 0XAB
#define SYNC2 0X3C
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