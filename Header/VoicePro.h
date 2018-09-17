#ifndef VOICE_PRO_H
#define VOICE_PRO_H

#include "public.h"

void VoicePro(void);
u16 Compare(u16 r,u16 w);
void Rs485Pro(void);
void Send485DataFromDMA(u8 *buf,u16 len);
void CheckCycleIndex(void);
void HandleRs485Data(u8 *buf,u8 len);

#endif

