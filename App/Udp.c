#include "NetPro.h"
#include "W5500.h"
#include "CanPro.h"
#include "Config.h"
#include "Udp.h"
#include "FmInit.h"

extern CNet Net;
extern vu16 SYS_TICK;


u8 Socket_Udp(u8 s)
{
     u8 status;
     u16 tick;
     WriteSocket2Byte(s,Sn_PORT, 5003);
     WriteSocket1Byte(s,Sn_MR,MR_UDP);
     WriteSocket1Byte(s,Sn_CR,OPENNET);
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     status = ReadSocket1Byte(s,Sn_SR);
     if(status != SOCK_UDP)
     {
          WriteSocket1Byte(s,Sn_CR,CLOSENET);
          return 0;
     }
     return 1;
}

