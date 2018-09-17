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
			if((MsTickDiff(Voice.Tick) > 1000) && (len <= 2000))     // ��ʱ�ж�
		     Voice.Status = Over;
               Voice.Status = Send;
               TIM2->CR1 |= 0x00000001;        // ����5ms��ʱ��
          }
          break;
     case Send:
          Rs485.Tick = SYS_TICK;          // �������ݷ����ڼ䣬������485��ѯ
		  Sys.BroadCastLeft = 20; 			//�������ݷ����ڼ䲻��鴫��������
          if(Voice.Dir == PcToDevice)     // PC�������ݵ��㲥�նˣ�����5ms DMA����ģʽ
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
               DMA1_Channel4->CCR |= 0x01;    //�������ݴ���		
               if(Voice.R == Voice.W)          // �����Ѿ��������
               {
                    Voice.Status = Over;
               }
          }
          else               // �㲥�ն���PC�������ݣ�ͨ��TCP�ϴ�
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
               if(Voice.R == Voice.W)    // �����Ѿ��������
               {
                    Voice.Status = Over;
				sendCnt = 0;
               }
          }
          break;
     case Over:
          TIM2->CR1 &= ~0x00000001;      // ֹͣ125ms ��ʱ��
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
	DMA1_Channel4 ->CMAR = (u32)&buf[0];//�洢����ַ
    DMA1_Channel4->CNDTR = len;
    DMA1_Channel4->CCR |= 0x01;    //�������ݴ���
	Rs485.Tick = SYS_TICK;
	
	fram = buf[0];
	fram <<= 8;
	fram += buf[1];
	fram <<= 8;
	fram += buf[2];
	fram <<= 8;
	fram += buf[3];
	SendDebugData(fram,&buf[4],len-4,2);//����Թ��߷���485�������
}

void CheckCycleIndex(void)
{
     u8 i;
     for(i = Rs485.CycleIndex+1;i < MaxDeviceNum;i ++)   // ������ ��Rs485.CycleIndex�������豸
     {
          if(Device[i].Name == 0x28)        // �㲥
          {
               Rs485.CycleIndex = i;
               return;
          }
     }
	if(Rs485.CycleIndex == 0xFF)
		return;
     for(i = 0;i <= Rs485.CycleIndex;i ++)   // ��������0��Rs485.CycleIndex �Ĺ㲥�豸
     {
          if(Device[i].Name == 0x28)        // �㲥
          {
               Rs485.CycleIndex = i;
               return;
          }
     }
     Rs485.CycleIndex = 0xFF;        // ���е��ˣ�˵���豸�б��в����ڹ㲥������ѯ
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
     Device[buf[4]-1].Status &= ~0x10;        // �㲥ͨѶ����
     BufClear(buf,MaxDmaLen);
     switch(buf[7])                           // ��ͬ�����ִ�в�ͬ�Ķ���
     {
          
     }
}

void Rs485Pro(void)
{
     switch(Rs485.Status)
     {
     case RS485Idle:
          if(MsTickDiff(Rs485.Tick) >= 5000)        // 5S��ѯһ��
          {
               Rs485.Tick = SYS_TICK;
               CheckCycleIndex();
               if(Rs485.CycleIndex == 0xFF)        // û�й㲥�豸���򲻲�����ѯ
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
