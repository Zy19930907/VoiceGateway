#ifndef VOICE_H
#define VOICE_H

#include <stm32f10x_lib.h>

#include "public.h"


void VoiceInit(void);
void ADCInit(void);
void DACInit(void);
void DMAForAdcInit(void);
void DMAForDacInit(void);
u16 GetAdc(u8 ch);
void Timer6Init(void);


#endif

