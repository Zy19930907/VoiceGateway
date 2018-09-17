#ifndef __PEKINGPOWER_H
#define __PEKINGPOWER_H

#include "Public.h"

typedef struct
{
	u8 Addr;
	u8 BatInfo[39];
	u8 W;
}_Pekingower;


u8 CheckPkPowerIndex(u8 addr);
u8 FilterPkPowerAddr(u8 addr);
void WritePkPowerRecord(u8 addr, u8 *buf,u8 len);
u16 ReadPekingPowerRecord(u8 *buf,u8 addr);
u16 ReadAllPekingPowerRecord(u8 *buf);

#endif
