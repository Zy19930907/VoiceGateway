#ifndef CPU_H
#define CPU_H

#include <stm32f10x_lib.h>

#include "public.h"

// PC9

#define RunLedOn     GPIOC ->ODR |= 0x00000200
#define RunLedOff    GPIOC ->ODR &= 0xFFFFFDFF
#define RunLedChange GPIOC ->ODR ^= 0x00000200

#define RS485_BAUD   45000

void CpuInit(void);
void Timer2Init(unsigned int arr,unsigned int psc);
void Timer3Init(unsigned int arr,unsigned int psc);
void Timer4Init(unsigned int arr,unsigned int psc);
void ClockInit(void);
void LedIoInit(void);
void VoiceIoInit(void);
void IWDG_Init(u8 prer,u16 rlr);
void IWDG_Feed(void);


#endif
