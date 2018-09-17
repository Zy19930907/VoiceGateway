#include "VoicePro.h"
#include "Public.h"
#include "Uart.h"
#include "W5500.h"
#include "NetPro.h"

CVoice Voice;
CRs485 Rs485;
extern CNet Net;
extern vu8 Timer125MsFlag;
extern vu16 SYS_TICK;
extern CSys Sys;
extern CSensor Device[MaxDeviceNum];

u16 sendCnt = 0;
u16 Compare(u16 r,u16 w)
{
     if(r <= w)
          return w - r;
     else
          return MaxVoiceLen - r + w;
}

void VoicePro(void)
{
     u16  i;
     u16 shift;
	u16 len;

	
     if(Voice.W >= Voice.R)
          len = Voice.W - Voice.R;
     else
          len = (MaxVoiceLen - Voice.R + Voice.W);
	
     switch(Voice.Status)
     {
     case Idle:
          if(Voice.W == 0)
               Voice.Tick = SYS_TICK;
          else
          {
               if(Voice.Dir == PcToDevice)
                    SetRs485Send;
			if((MsTickDiff(Voice.Tick) > 1000) && (len <= 2000))     // 超时判断
		     Voice.Status = Over;
               Voice.Status = Send;
               TIM2->CR1 |= 0x00000001;        // 触发5ms定时器
          }
          break;
     case Send:
          Rs485.Tick = SYS_TICK;          // 语音数据发起期间，不参与485轮询
		  Sys.BroadCastLeft = 20; 			//语音数据发起期间不检查传感器断线
          if(Voice.Dir == PcToDevice)     // PC发送数据到广播终端，触发5ms DMA发送模式
          {
               if(Timer125MsFlag == 0)
                    break;
               Timer125MsFlag = 0;
               shift = Compare(Voice.R,Voice.W);
               if(shift >= 500)
                    shift = 500;
			Rs485.Buf[0] = 0x55;
			Rs485.Buf[1] = 0xAA;
			Rs485.Buf[2] = 0x55;
			Rs485.Buf[3] = 0xAA;
			Rs485.Buf[4] = Voice.DestAddr;
               for(i = 0;i < shift;i ++)
               {
                    Rs485.Buf[i+5] = Voice.RecBuf[Voice.R++];
                    if(Voice.R == MaxVoiceLen)
                         Voice.R = 0;
               }
			
               DMA1_Channel4->CCR &= ~0x01;
				DMA1_Channel4->CMAR = (u32)&Rs485.Buf[0];
               DMA1_Channel4->CNDTR = shift+5;
               DMA1_Channel4->CCR |= 0x01;    //开启数据传输		
               if(Voice.R == Voice.W)          // 数据已经发送完成
               {
                    Voice.Status = Over;
               }
          }
          else               // 广播终端向PC发送数据，通过TCP上传
          {
               if(Timer125MsFlag == 0)
                    break;
               Timer125MsFlag = 0;
			Net.Buf[0] = 0x55;
			Net.Buf[1] = 0xAA;
			Net.Buf[2] = 0x55;
			Net.Buf[3] = 0xAA;
               Net.Buf[4] = Voice.DestAddr;
			Net.Buf[5] = sendCnt;
			Net.Buf[6] = sendCnt >> 8;
               for(i = 0;i < 500;i ++)
               {
                    Net.Buf[i+7] = Voice.RecBuf[Voice.R++];
                    if(Voice.R == MaxVoiceLen)
                         Voice.R = 0;
               }
			SendSocketData(Sys.VoiceSocket,&Net.Buf[0],507);
			sendCnt++;
               if(Voice.R == Voice.W)    // 数据已经发送完成
               {
                    Voice.Status = Over;
				sendCnt = 0;
               }
          }
          break;
     case Over:
          TIM2->CR1 &= ~0x00000001;      // 停止125ms 定时器
          Voice.W = 0;
          Voice.R = 0;
          SetRs485Rec;
          Voice.Status = Idle;
          break;
     }
}

void Send485DataFromDMA(u8 *buf,u16 len)
{
	u32 fram;
    DMA1_Channel4->CCR &= ~0x01;
	DMA1_Channel4 ->CMAR = (u32)&buf[0];//存储器地址
    DMA1_Channel4->CNDTR = len;
    DMA1_Channel4->CCR |= 0x01;    //开启数据传输
	Rs485.Tick = SYS_TICK;
	
	fram = buf[0];
	fram <<= 8;
	fram += buf[1];
	fram <<= 8;
	fram += buf[2];
	fram <<= 8;
	fram += buf[3];
	SendDebugData(fram,&buf[4],len-4,2);//向调试工具发送485检测数据
}

void CheckCycleIndex(void)
{
     u8 i;
     for(i = Rs485.CycleIndex+1;i < MaxDeviceNum;i ++)   // 先搜索 从Rs485.CycleIndex到最后的设备
     {
          if(Device[i].Name == 0x28)        // 广播
          {
               Rs485.CycleIndex = i;
               return;
          }
     }
	if(Rs485.CycleIndex == 0xFF)
		return;
     for(i = 0;i <= Rs485.CycleIndex;i ++)   // 在搜索从0到Rs485.CycleIndex 的广播设备
     {
          if(Device[i].Name == 0x28)        // 广播
          {
               Rs485.CycleIndex = i;
               return;
          }
     }
     Rs485.CycleIndex = 0xFF;        // 运行到此，说明设备列表中不存在广播，则不轮询
}

void Make485FramHeader(void)
{
     Rs485.Buf[0] = 0x55;
     Rs485.Buf[1] = 0xAA;
     Rs485.Buf[2] = 0x55;
     Rs485.Buf[3] = 0xAA;
}

void MakeCycleBuf(u8 addr)
{
     u16 t,crc;
     t = 0;
     Rs485.CmdBuf[t++] = 0x55;
     Rs485.CmdBuf[t++] = 0xAA;
     Rs485.CmdBuf[t++] = 0x55;
     Rs485.CmdBuf[t++] = 0xAA;
     Rs485.CmdBuf[t++] = addr;
     Rs485.CmdBuf[t++] = 0x01;
     Rs485.CmdBuf[t++] = 0x00;  
     Rs485.CmdBuf[t++] = 0x00;  
     crc = CalCrc16(&Rs485.CmdBuf[4],4,0);
     Rs485.CmdBuf[t++] = crc;
     Rs485.CmdBuf[t++] = crc >> 8;     
     Rs485.Cnt = t;
}


void HandleRs485Data(u8 *buf,u8 len)
{
	u32 fram;
	
	fram = buf[0];
	fram <<= 8;
	fram += buf[1];
	fram <<= 8;
	fram += buf[2];
	fram <<= 8;
	fram += buf[3];
	
	SendDebugData(fram,&buf[4],len-4,2);
	
     if((buf[0] != 0x55) || (buf[1] != 0xAA) || (buf[2] != 0x55) || (buf[3] != 0xAA))
          return;
     if(!CRC16Check(&buf[4],len - 4))
          return;
     Device[buf[4]-1].CheckCnt = 0;
     Device[buf[4]-1].Status &= ~0x10;        // 广播通讯正常
     BufClear(buf,MaxDmaLen);
     switch(buf[7])                           // 不同的命令，执行不同的动作
     {
          
     }
}

void Rs485Pro(void)
{
     switch(Rs485.Status)
     {
     case RS485Idle:
          if(MsTickDiff(Rs485.Tick) >= 5000)        // 5S轮询一次
          {
               Rs485.Tick = SYS_TICK;
               CheckCycleIndex();
               if(Rs485.CycleIndex == 0xFF)        // 没有广播设备，则不参与轮询
                    break;
               Rs485.Status = RS485Cycle;
			   if(Device[Rs485.CycleIndex].CheckCnt>=3)
				   Device[Rs485.CycleIndex].Status |= 0x10;
			   if(Device[Rs485.CycleIndex].CheckCnt<3)
					Device[Rs485.CycleIndex].CheckCnt ++;
          }
          break;
     case RS485Cycle:
          MakeCycleBuf(Rs485.CycleIndex+1);
          Rs485.Tick = SYS_TICK;
          Rs485.Status = RS485SendStep1;
          break;
     case RS485Receive:
          HandleRs485Data(Rs485.Buf,Rs485.Cnt);
	      Rs485.Status = RS485Idle;
          break;
     case RS485SendStep1:
          SetRs485Send;
          if(MsTickDiff(Rs485.Tick) >= 10)
          {
			Send485DataFromDMA(Rs485.CmdBuf,Rs485.Cnt);
			Rs485.Status = RS485SendStep2;
          }
          break;
     case RS485SendStep2:
          if(DMA1_Channel4->CNDTR <= 1)   
		{
			Rs485.Tick = SYS_TICK;
			Rs485.Status = RS485SendStep3;
		}
          break;
     case RS485SendStep3:
		if(MsTickDiff(Rs485.Tick) >= 8)
          {
			SetRs485Rec;
			Rs485.Tick = SYS_TICK;
			Rs485.Status = RS485Idle;
		}
          break;
     }
}
