#ifndef CONFIG_H
#define CONFIG_H

#include "Public.h"

void ReadConfig(void);
void ReadDefaultAddr(void);
void ReadDefaultIpAddr(void);
void ReadDefaultGateWayIp(void);
void ReadDefaultResetTimes(void);
void WriteConfig(u16 addr,u8 len);
void ReadDefaultMAC(void);


#endif
