#ifndef __SWITCHER_H
#define __SWITCHER_H

#include "Public.h"
#define MaxSwitcher		8

typedef struct
{
	u8 Addr;
	u8 Flag;
	u16 Tick;
}_Switcher;

u8 FilterSwitcher(u8 Addr);
void SwitchersInit(void);
void SwitcherOnlineCheck(void);

#endif
