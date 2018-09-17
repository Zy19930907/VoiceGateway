#ifndef   MX25L128_H
#define	MX25L128_H

#include "public.h"


/*
   PB0----CS
   PB1----MISO
   PB10----MOSI
   PB11----SCLK
*/

#define SET_SCLK     GPIOB -> ODR |=  0x0800
#define CLR_SCLK     GPIOB -> ODR &= ~0x0800

#define SET_MOSI     GPIOB -> ODR |=  0x0400
#define CLR_MOSI     GPIOB -> ODR &= ~0x0400

#define SET_CS       GPIOB -> ODR |=  0x0001
#define CLR_CS       GPIOB -> ODR &= ~0x0001

#define MISO        (GPIOB -> IDR & 0x0002)








#endif

