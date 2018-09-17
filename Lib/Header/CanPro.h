#ifndef CAN_PRO_H
#define CAN_PRO_H

#include "public.h"

void CanPro(void);
void UpdateInfo(void);
u32 MakeFramID(u8 Cmd,u8 DestAddr);
void SendDataCan(u32 ID,u8 *buf,u8 len);



#endif

