#ifndef CPU_H
#define CPU_H

#include <stm32f10x_lib.h>

#include "public.h"

// PA3

#define RunLedOn     GPIOA ->ODR |= 0x00000008
#define RunLedOff    GPIOA ->ODR &= 0xFFFFFFF7
#define RunLedChange GPIOA ->ODR ^= 0x00000008

#define Check_HuiCai (GPIOA ->IDR & 0x0100)

#define RelayOn      GPIOC -> ODR |=  0x00000100
#define RelayOff     GPIOC -> ODR &= ~0x00000100 

#define BellOff       GPIOC -> ODR |=  0x00000200
#define BellOn      GPIOC -> ODR &= ~0x00000200 
#define BellChange  GPIOC -> ODR ^=  0x00000200


void CpuInit(void);
void Timer2Init(unsigned int arr,unsigned int psc);
void Timer3Init(unsigned int arr,unsigned int psc);
void ClockInit(void);
void LedIoInit(void);
void VoiceIoInit(void);
void IWDG_Init(u8 prer,u16 rlr);
void IWDG_Feed(void);


#endif
