#ifndef __DEVICEMANGER_H
#define __DEVICEMANGER_H

#include "Public.h"
#include "IPD_ZJM.h"
#include "PekingPower.h"
#include "Switcher.h"

typedef struct{
	void (*WriteRecord_IPD_ZJM)(u8 byte,u8 *buf,u32 id,u8 len);
	void (*WriteRecord_PekingPower)(u8 byte,u8 *buf,u8 len);
	u16 (*ReadAllRecord_PekingPower)(u8 *buf);
	u16 (*ReadRecord_PekingPower)(u8 *buf,u8 addr);
	void (*ReadRecord_IPD_ZJM)(u8 *buf,u8 addr);
	u16 (*ReadDetailRecord_IPD_ZJM)(u8 *buf,u8 addr);
	void (*ReadIPDSetValue)(u8 *buf,u8 addr);
	u8 (*GetSwitcher)(u8 Addr);
	void (*SwitcherOnlineCheck)(void);
}_DeviceManger;

extern _DeviceManger DeviceManger;

void DeviceMangerInit(void);

#endif
