

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SHT21_H
#define	SHT21_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include "i2c_functions.h"

int SHT2x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum);
int SHT21_Read_user_register();
int SHT21_Set_user_register();
unsigned int SHT21_Read_Temperature();
unsigned int SHT21_Read_Humidity();
int SHT21_init(void);

#endif	/* XC_HEADER_TEMPLATE_H */

