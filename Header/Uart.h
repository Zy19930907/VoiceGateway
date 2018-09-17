#ifndef UART_H
#define UART_H

#include "Public.h"

#define SetRs485Send     GPIOA ->ODR |= 0x00000100
#define SetRs485Rec      GPIOA ->ODR &= 0xFFFFFEFF


void UartInit(u32 pclk2,u32 bound);
void Dma1Init(void);
void Dma2Init(void);



#endif
