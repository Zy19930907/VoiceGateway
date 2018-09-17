#include <stm32f10x_lib.h>
#include "public.h"
#include "CPU.h"
#include "NetPro.h"
#include "CanPro.h"
#include "VoicePro.h"
#include "Config.h"
#include "CanPro.h"
#include "DeviceManger.h"

CRunLevel RunLevel;
u16 LedTick,LedRunTime,UpdateTick,LianDongTick;
extern vu16 SYS_TICK;
extern CSys Sys;

void FlashRunLed(u8 times)
{
     u16 tick;
     while(times)
     {
          RunLedOn;
          tick = SYS_TICK;
          while(MsTickDiff(tick) < 50);
          RunLedOff;
          tick = SYS_TICK;
          while(MsTickDiff(tick) < 50);
          times--;
     }
}

int main(void)
{
//	SCB->VTOR = 0x08010000;
	CpuInit();
	SYS_TICK = 0;
	while(MsTickDiff(LedTick) < 1000);
	ReadDefaultConfig();
	LedRunTime = 500;
	Sys.Delay = 5;
	Sys.SyncLeft = 5;
	Sys.InitLeft = 5;
	Sys.DebugSocket = 0xFF;
	Sys.ActFilterTime = 3;
	DeviceMangerInit();
	IWDG_Init(3,625*3);   //1500 ms

     while(1)
     {
          switch(RunLevel)
          {
          case LedLevel:
               if(MsTickDiff(LedTick) > LedRunTime)
               {
                    LedTick = SYS_TICK;
                    RunLedChange;
                    IWDG_Feed();
               }
               RunLevel ++;
               break;
          case CanLevel:
               CanPro();
               RunLevel ++;
               break;
          case InitLevel:
               CanSendInitInfo();
               RunLevel ++;
		     break;
          case LianDongLevel:
               if(MsTickDiff(LianDongTick) > 1000)     // 有联动触发时，1S执行一次
               {
                    LianDongTick = SYS_TICK;
                    CheckTriggerStatus();
                    SendLianDong();
               }
               RunLevel ++;
		     break;
          case VoiceLevel:
               VoicePro();
               RunLevel ++;
               break;
          case NetLevel:
               NetPro();
               RunLevel ++;
               break;
		  
          case Rs485Level:
               Rs485Pro();
               RunLevel ++;
               break;
		  
          case UpdateDevice:
               if(MsTickDiff(UpdateTick) > 1000)
               {
                    UpdateTick = SYS_TICK;
                    UpdateDeviceInfo();
			     DeviceManger.SwitcherOnlineCheck();//转换板断线处理
                    if(Sys.Delay)
                         Sys.Delay --;
               }
               RunLevel = LedLevel;
               break;
			   
          default:
               RunLevel = LedLevel;
               break;
          }
     }
}
