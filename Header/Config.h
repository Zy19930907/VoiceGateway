#ifndef CONFIG_H
#define CONFIG_H

#include "Public.h"

#include "Public.h"


void ReadFactoryTime(void);
u8  ConfigCheck(u8 addr,u8 len);
void WriteConfig(u8 addr,u8 len);
void ReadDefaultAddr(void);
void ReadDefaultIpAddr(void);
void ReadDefaultGateWayIp(void);
void ReadDefaultResetTimes(void);
void ReadDefaultConfig(void);
void ReadDefaultInit(void);
void ReadDefaultReadPtr(void);
void ReadDefaultWritePtr(void);
void UpdateRptr(void);
void UpdateWptr(void);
void ReadCommunication(void);
void UpdateDeviceInfo(void);
u8 ReadRecord(u16 addr,u8 *buf);
void ReadInitInfo(u8 addr);
void UpdatePowerInfo(u8 addr);
void ReadDefaultMAC(void);
void ReadDefaultFilterTime(void);
void UpdateFilterTime(void);
void ReadDefaultCrcCnt(void);
void ReadDefaultUdp(void);

#endif	/* DEFAULTCONFIG_H */

