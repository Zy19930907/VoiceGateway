#include "CanPro.h"
#include "Can.h"
#include "SD2068.h"
#include "W5500.h"
#include "Config.h"
#include "NetPro.h"
#include "FmInit.h"
#include "DeviceManger.h"

CCan   Can;
CAckAddr Ack;
CSendInit Init;
CLianDong LianDong;
extern CanRxMsg RxMessage;
extern CExcute Excute[MaxTriggerNum];
extern CPower Power[MaxPowerNum];
extern CSensor Device[MaxDeviceNum];
extern vu16 SYS_TICK,SYS_TickSec;
extern CSys  Sys;
extern CTime Time;
extern CNet Net;
extern _Switcher Switchers[MaxSwitcher];

// 查看是否有联动
void CheckTriggerStatus(void)
{
     u8 i,warn;  
     warn = 0;
     Sys.PauseVoice = 0;
     for(i = 0;i < MaxTriggerNum;i ++)
     {
          warn |=  Excute[i].Warn;
          if(Excute[i].ExcuteAddr != Excute[i+1].ExcuteAddr)
          {
               Device[Excute[i].ExcuteAddr - 1].Warn = warn;
               if(warn)              // 证明有联动发生
                    Sys.PauseVoice = 1;
               warn = 0;
          }
     }
}

// style : 0x10 断线  0x01 上限断电  0x20  0x40 开关量0和1关联
void CheckLianDongInfo(u8 addr,u8 style,u8 val)
{
     u8 i,flag,j,trigger;
     flag = 0;
     if(!(Device[addr - 1].Flag & 0x10))        //参与联动的设备
          return;
     for(i = 0;i < MaxTriggerNum;i ++)
     {
          if(Excute[i].TriggerAddr == 0)
          {
               if(flag == 0)
                    return;
               else
                    break;
          }
          if(Excute[i].TriggerAddr == addr)
          {
               j = Excute[i].ExcuteAddr - 1;
               if(Device[j].Name == DuanDianQi)
                    continue;
               trigger = 0;
               switch(style)
               {
               case 0x10:           // 断线
                    if(Excute[i].Trigger & 0x10)
                         trigger = 0x10;
                    break;
               case 0x01:           // 模拟量上断电
                    if(Excute[i].Trigger & 0x01)
                         trigger = 0x01;
                    break;
               default:             // 开关量
                    if(Excute[i].Trigger & 0x20)       // 0态
                         trigger = 0x20;
                    if(Excute[i].Trigger & 0x40)       // 1态
                         trigger = 0x40;
                    break;
               }
               if(val == 0)
                    Excute[i].Warn &= ~trigger;
               else
                    Excute[i].Warn |= trigger;
          }
     }
}


u8 CheckPowerIndex(u8 addr)
{
     u8 i;
     for(i = 0;i < MaxPowerNum;i ++)
     {
          if(addr == Power[i].Addr)
               return i;
     }
     return 0xFF;
}

u8 FilterPowerAddr(u8 addr)
{
     u8 j;
     if(!(Device[addr-1].Flag & 0x01))
          return 0xFF;
     for(j = 0;j < MaxPowerNum;j ++)
     {
          if(addr == Power[j].Addr)
               return j;
     }

     for(j = 0;j < MaxPowerNum;j ++)
     {
          if(Power[j].Addr == 0)
               return j;
     }
     return 0xFF;
}

u8 MakeCanBusStatus(u8 CanIndex)
{
     u8 status;
     switch(CanIndex)
     {
     case 1:status = 0x00;break;
     case 2:status = 0x20;break;
     case 3:status = 0x40;break;
     case 4:status = 0x60;break;
     default :status = 0x00;break;
     }
     return status;
}


u8 CanBusBelong(u8 addr)
{
     u8 n;
     if(!(Device[addr-1].Flag & 0x03))
          return 0;
     n = (Device[addr-1].Status & 0x60);
     n >>= 5;
     return (n+1);
}

void IpdControlDeal(u8 *buf,u8 index,u8 addr)
{
	u8 i;
	u8 IpdSetBuf[24];
	u16 CtrFlag,temp;
	CtrFlag = buf[3];
	CtrFlag <<= 8;
	CtrFlag |= buf[2];

	temp = (CtrFlag >> 1);
	for(i=0;i<3;i++)//复位、分闸、合闸处理
	{
		if(temp & 0x01)
		{
			Can.ID = MakeFeimoCanId(4, 0x55, 0, 0, buf[1], buf[0]);
			BufClear(Can.Buf,8);
			switch(i)
			{
				case 0://分闸
					Can.Buf[4] = 0xFF;
					Can.Buf[5] = 0xFF;
					break;
				case 1://合闸
					Can.Buf[2] = 0xFF;
					Can.Buf[3] = 0xFF;
					break;
				case 2://复位
					Can.Buf[0] = 0xFF;
					Can.Buf[1] = 0xFF;
					break;
			}
			CanSendData(index, Can.ID, Can.Buf,8);
			break;
		}
		temp >>= 1;
	}
	temp = CtrFlag >> 4;
	DeviceManger.ReadIPDSetValue(IpdSetBuf,addr);
	for(i=0;i<3;i++)
	{
		if(temp & 0x01)
		{
			switch(i)
			{
				case 0:
					Can.ID = MakeFeimoCanId(3, 0x55, 0, 0, buf[1], buf[0]);
					BufCopy(Can.Buf,&buf[4],8);
					break;
				case 1:
					Can.ID = MakeFeimoCanId(2, 0x55, 0, 0, buf[1], buf[0]);
					BufCopy(Can.Buf,&buf[12],8);
					break;
				case 2:
					Can.ID = MakeFeimoCanId(1, 0x55, 0, 0, buf[1], buf[0]);
					BufCopy(Can.Buf,&buf[20],8);
					break;
			}
			CanSendData(index, Can.ID, Can.Buf,8);
		}
		temp >>= 1;
	}
}

void NetInfoToCanOrRs485(u8 cmd,u8 *buf,u8 len)
{
     u8 i,index,t[8];
     /*if(Device[buf[0]-1].Name == 0x28)      // 广播需要转发到语音线,后续协议待定
     {

     }*/
	index = CanBusBelong(buf[0]); 
	Ack.NeedToNet = 1;
     Ack.Addr = buf[0];
     Ack.Tick = SYS_TICK;
	if (cmd == 0x55)
		IpdControlDeal(buf,index,buf[0]);
     else
     {
         
          Can.ID = (u32)MakeFramID(0,cmd,buf[0]);
          for(i = 1;i < len;i ++)
               t[i-1] = buf[i];
          CanSendData(index,Can.ID,t,len-1);  
		SendDebugData(Can.ID,t,len-1,3);
     }          
}

u32 MakeFramID(u8 Fram,u8 Cmd,u8 DestAddr)
{
     u32 ID;
     ID = 0;
     ID = Fram;
     ID <<= 7;
     ID += Cmd;
     ID <<= 17;
     ID += DestAddr;
     return ID;
}

void TimeChange(u8 *buf)
{
     u32 timeHex;
     timeHex = Time.Buf[0];    //?
     timeHex <<= 4;
     timeHex += Time.Buf[1];   //?
     timeHex <<= 5;
     timeHex += Time.Buf[2];   //?
     timeHex <<= 5;
     timeHex += Time.Buf[4];   //??
     timeHex <<= 6;
     timeHex += Time.Buf[5];   //?
     timeHex <<= 6;
     timeHex += Time.Buf[6];   //?
     buf[0] = timeHex;
     buf[1] = timeHex >> 8;
     buf[2] = timeHex >> 16;
     buf[3] = timeHex >> 24;
}

u8 CheckCanTxBufMcp2515(u8 index)
{
     u8 flag;
     flag = ReadRegCan(index,TXB0CTRL);
     if(!(flag & 0x08))
          return 0;
     flag = ReadRegCan(index,TXB1CTRL);
     if(!(flag & 0x08))
          return 1;
     flag = ReadRegCan(index,TXB2CTRL);
     if(!(flag & 0x08))
          return 2;
     return 0xFF;
}
//index 2...4
void CanMcp2515RecFunc(u8 index)
{
     u8 flag;
     flag = ReadRegCan(index,CANINTF);
     if(flag & 0x01)
     {
          Can.Tick[index-1] = SYS_TICK;
          Can.Len = ReadRegCan(index,RXB0DLC);
		if(Can.Len >= 8)
			Can.Len = 8;
          Can.ID = (ReadRegCan(index,RXB0SIDL) & 0x03);
          Can.ID <<=8;
          Can.ID += ReadRegCan(index,RXB0EIDH);
          Can.ID <<= 8;
          Can.ID += ReadRegCan(index,RXB0EIDL);
          Can.ID <<= 8;
          Can.ID += ReadRegCan(index,RXB0SIDH);
          Can.ID <<= 3;
          Can.ID += (ReadRegCan(index,RXB0SIDL) >> 5);
          ReadBurstRegCan(index,RXB0D0,&Can.Buf[0],Can.Len);
          HandleCanData(Can.ID,index);
          ModifyReg(index,CANINTF,0x21,0x00);
    }
    if(flag & 0x02)
    {
          Can.Tick[index-1] = SYS_TICK;
          Can.Len = ReadRegCan(index,RXB1DLC);
		if(Can.Len >= 8)
			Can.Len = 8;
          Can.ID = (ReadRegCan(index,RXB1SIDL) & 0x03);
          Can.ID <<=8;
          Can.ID += ReadRegCan(index,RXB1EIDH);
          Can.ID <<= 8;
          Can.ID += ReadRegCan(index,RXB1EIDL);
          Can.ID <<= 8;
          Can.ID += ReadRegCan(index,RXB1SIDH);
          Can.ID <<= 3;
          Can.ID += (ReadRegCan(index,RXB1SIDL) >> 5);
          ReadBurstRegCan(index,RXB1D0,&Can.Buf[0],Can.Len);
          HandleCanData(Can.ID,index);
          ModifyReg(index,CANINTF,0x22,0x00);
    }
}

void CanPro(void)
{
     u8 i;
	//u32 t;
     switch(Can.Status)
     {
     case INIT:
          Can1Init();
          Can2Init();
          Can3Init();
          Can.Status = RX;
          break;
     case RX:
		Can1RecvFunc();
	     CanMcp2515RecFunc(2);
		CanMcp2515RecFunc(3);
        for(i = 0;i < 3;i ++)
        {
			if(MsTickDiff(Can.Tick[i]) > 30000)
			{
				Can.Tick[i] = SYS_TICK;
				switch(i)
				{
			     case 0:Can1Init();break;
                    case 1:Can2Init();break;
                    case 2:Can3Init();break;
				}
			}
		}
          break;
     /*case RXOVER: 
		t = RxMessage.ExtId;
		Can.ID = (t >> 18) + ((t&0x3FFFF)<<11);
		BufCopy(&Can.Buf[0],&RxMessage.Data[0],RxMessage.DLC);
		Can.Len = RxMessage.DLC; 
          HandleCanData(Can.ID,1);
		Can.Status = RX;
		break;*/
     default:
          Can.Status = INIT;
          break;
     }
}

// 缓冲区0固定用于发送传感器实时数据，缓冲区1,2用于回复信息
u8 CheckCanTxBuf(void)
{
     u8 t;
     t = 0xFF;
     //if((CAN->TSR >> 26) & 0x01)   //发送缓冲区1空
     //{
         // t = 0;
         // return t;
     //}
     if((CAN->TSR >> 27) & 0x01)   //发送缓冲区2空
     {
          t = 1;
          return t;
     }
     if((CAN->TSR >> 28) & 0x01)   //发送缓冲区3空
     {
          t = 2;
          return t;
     }
     return t;
}

u8 SendDataCAN1(u32 ID,u8 *buf,u8 len)
{
      u8 i;
	i= CheckCanTxBuf();
	if(i > 2)
		return 0xFF;

	CAN->sTxMailBox[i].TIR = CanIdSwitch(ID,CANIDSWITCH_F2B);
	CAN->sTxMailBox[i].TDTR = len;
	CAN->sTxMailBox[i].TDHR=(((u32)buf[7]<<24)|
                                ((u32)buf[6]<<16)|
                                ((u32)buf[5]<<8)|
                                ((u32)buf[4]));
	CAN->sTxMailBox[i].TDLR=(((u32)buf[3]<<24)|
                                ((u32)buf[2]<<16)|
                                ((u32)buf[1]<<8)|
                                ((u32)buf[0]));
	CAN->sTxMailBox[i].TIR |= 0x00000001; //启动发送
	return i;
}


void SendDataCAN23(u8 index,u32 ID,u8 *buf,u8 len)
{
     u8 t,RegShift;
     t = CheckCanTxBufMcp2515(index);
     if(t == 0xFF)
          return;
     switch(t)
     {
     case 0: RegShift = 0x00;break;
     case 1: RegShift = 0x10;break;
     case 2: RegShift = 0x20;break;
     }
     WriteRegCan(index,TXB0EIDH+RegShift, ID>>19);
     WriteRegCan(index,TXB0EIDL+RegShift, ID>>11);
     WriteRegCan(index,TXB0SIDH+RegShift, ID>>3);
     WriteRegCan(index,TXB0SIDL+RegShift, (ID<<5)+0x08+(ID >> 27));
     WriteRegCan(index,TXB0DLC+RegShift, len); //?????0?????
     WriteBurstRegCan(index,TXB0D0+RegShift,buf,len);
     ModifyReg(index,TXB0CTRL+RegShift,0x08,0x08);
}

void CanSendData(u8 index,u32 ID,u8 *buf,u8 len)
{
     switch(index)
     {
     case 1:
          SendDataCAN1(ID,buf,len);
          break;
     case 2:
     case 3:
          SendDataCAN23(index,ID,buf,len);
          break;
     }
}


void CanSendInitInfo(void)
{
     u8 i,buf[8];
	
     switch(Init.Status)
     {
     case INITIDLE:
          if(Init.W == 0)
               break;
          if(Init.W <= 8)
               Init.FramStatus = F_Single;
          else
               Init.FramStatus = F_First;
          Init.FramCnt = 0;
          Init.Status = SENDPREPARE;
          break;
     case SENDPREPARE:
          if(Init.FramStatus == F_Single)
          {
               Can.ID = MakeFramID(0,0x51,Init.DestAddr);
			SendDebugData(0,Init.Buf,Init.W,1);
               Init.Shift = Init.W;
          }
          if(Init.FramStatus == F_First)
          {
			SendDebugData(0,Init.Buf,Init.W,1);
               Can.ID = MakeFramID(0x04+Init.FramCnt,0x51,Init.DestAddr);
               Init.Shift = 8;
          }
          if(Init.FramStatus == F_Middle)
          {
               Can.ID = MakeFramID(0x08+Init.FramCnt,0x51,Init.DestAddr);
               Init.Shift = 8;
          }
          if(Init.FramStatus == F_End)
          {
               Can.ID = MakeFramID(0x0C+Init.FramCnt,0x51,Init.DestAddr);
               Init.Shift = Init.W - Init.R;
          }
          for(i = 0;i < Init.Shift;i ++)
               buf[i] = Init.Buf[Init.R + i];
          Init.Tick = SYS_TICK;
          while(MsTickDiff(Init.Tick) < 20);
          Init.Status = SENDING;
          Init.Tick = SYS_TICK;
          CanSendData(CanBusBelong(Init.DestAddr),Can.ID,buf,Init.Shift);
		SendDebugData(Can.ID,buf,Init.Shift,3);
          Init.Retry ++;
          break;
     case SENDING:
          if(Init.Retry > 5)
          {
               Init.Retry = 0;
               Init.Status = INITIDLE;
               Init.W = 0;
               Init.R = 0;
          }
          if(MsTickDiff(Init.Tick) > 1500)       //1.5s??,??
               Init.Status = INITIDLE;  
          break;
     default:
          Init.Status = INITIDLE;
          break;
     }
}


void AckFunc(u8 index,u8 cmd,u8 DestAddr)
{
     u8 buf[1];
//     Can.ID =(u32) MakeFramID(0,cmd,DestAddr);
//     CanSendData(index,Can.ID,&Can.Buf[0],0);
     
     Can.ID =(u32) MakeFramID(0,cmd,DestAddr);
     if(Device[DestAddr-1].Flag & 0x04)
          Can.ID |= 0x8000;
     buf[0] = Device[DestAddr-1].Warn & 0x7F;
     if(Sys.Delay)
          CanSendData(index,Can.ID,&buf[0],0);
     else
          CanSendData(index,Can.ID,&buf[0],1);
}


void CheckCrc(u8 addr,u8 crc,u8 flag)
{
     u16 tick;
     if(Sys.InitLeft)
          return;
     if(Init.Status != INITIDLE)
          return;
     if(Device[addr-1].CrcCnt > 5)
          return;
     if(crc == Device[addr-1].Crc)
     {
          Device[addr-1].CrcCnt = 0;
          return;
     }
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 20);
     Device[addr-1].CrcCnt ++;
     if(Device[addr-1].Crc == 0)
     {
          Can.ID =(u32) MakeFramID(0,0x52,addr);
          CanSendData(CanBusBelong(addr),Can.ID,&Can.Buf[0],0);
          return;
     }
     else
     {
          Init.R = 0;
          Init.W = 0;
          if(!flag)
               Init.Buf[Init.W++] = Device[addr-1].Crc;
          else
               ReadInitInfo(addr);
     }
     Init.DestAddr = addr;
}


void SendLianDong(void)
{
	u8 i;
     u8 buf[1];
     for(i = LianDong.Shift;i < MaxDeviceNum;i ++)
     {
          if((Device[i].Warn) && (!(Device[i].Status & 0x80)))        // 需要发送联动，并且处于正常
          {
               Can.ID = (u32) MakeFramID(0,0x39,i+1);
               buf[0] = Device[i].Warn & 0x7F;
               CanSendData(CanBusBelong(i+1),Can.ID,&buf[0],1);
               LianDong.Shift = i;
               LianDong.Shift ++;
               if(LianDong.Shift == MaxDeviceNum)
                    LianDong.Shift = 0; 
               return;
          }
     }
     for(i = 0;i < LianDong.Shift;i ++)
     {
          if((Device[i].Warn) && (!(Device[i].Status & 0x80)))        // 需要发送联动，并且处于正常
          {
               Can.ID = (u32) MakeFramID(0,0x39,i+1);
               buf[0] = Device[i].Warn & 0x7F;
               CanSendData(CanBusBelong(i+1),Can.ID,&buf[0],1);
               LianDong.Shift = i;
               LianDong.Shift ++;
               if(LianDong.Shift == MaxDeviceNum)
                    LianDong.Shift = 0; 
               return;
          }
     }
}


//发送3.0断电配置信息
void SendBreak3_0(void)
{
	u8 i;
	Can.Len = 0;
	Can.ID = MakeFramID(0,0x57,0);
	Can.ID |= (1 << 15);
	for(i=0;i<Breaks3_0.Cnt;i++)
	{
		Can.Buf[2*i] = Breaks3_0.CH4Addr[i];
		Can.Buf[2*i+1] = Breaks3_0.BreakerAddr[i];
		Can.Len+=2;
	}
	for(i=1;i<4;i++)
		CanSendData(i,Can.ID,&Can.Buf[0],Can.Len);
}

void MakeSensorCanData(u32 ID,u8 CanIndex,u8 addr)
{
     u8 status;
     status = 0;
     status += ((Can.Buf[1] >> 3) & 0x03);     //??
     status <<= 2;
     status += ((Can.Buf[1] >> 5) & 0x03);     //???
     status &= 0x0F;
     status += MakeCanBusStatus(CanIndex);
     
     Device[addr-1].Status = status;                  //???
     
//     if(ID & 0x00010000)    // D16: 本来是断电测试，但是针对张家峁的北京院传感器，无此功能
//          Device[addr-1].Status |= 0x10; 
     
     if((Device[addr-1].Buf[0] !=  Can.Buf[0]) || (Device[addr-1].Buf[1] != (Can.Buf[1] & 0x07)))
     {
          Device[addr-1].Buf[0] =  Can.Buf[0];
          Device[addr-1].Buf[1] = (Can.Buf[1] & 0x07);
          Device[addr-1].Buf[2] = 0;
          if(Sys.Flag & 0x02)                  // 与上位机通讯中断，则存储实时数据
               WriteSensorRecord(addr,SENSORVARY);
     }
}
// 关联梅安森公司的 CH4 O2 CO WD
void MakeMASWirelessSensor(u8 CanIndex,u8 addr)
{     // CH4
     Device[addr-1].Tick =  SYS_TickSec;
     Device[addr-1].Name =  26;
     Device[addr-1].Status = MakeCanBusStatus(CanIndex) + 0x02;
     Device[addr-1].Buf[0] =  Can.Buf[0];
     Device[addr-1].Buf[1] =  Can.Buf[1] & 0x07;
     Device[addr-1].Buf[2] = 0;
     // O2 
     Device[addr].Tick =  SYS_TickSec;
     Device[addr].Name =  27;
     Device[addr].Status = MakeCanBusStatus(CanIndex) + 0x01;
     Device[addr].Buf[0] =  Can.Buf[2];
     Device[addr].Buf[1] =  Can.Buf[3] & 0x07;
     Device[addr].Buf[2] = 0;
    // CO 
     Device[addr+1].Tick =  SYS_TickSec;
     Device[addr+1].Name =  28;
     Device[addr+1].Status = MakeCanBusStatus(CanIndex) + 0x00;
     Device[addr+1].Buf[0] =  Can.Buf[4];
     Device[addr+1].Buf[1] =  Can.Buf[5] & 0x07;
     Device[addr+1].Buf[2] = 0;
    // WD 
     Device[addr+2].Tick =  SYS_TickSec;
     Device[addr+2].Name =  29;
     Device[addr+2].Status = MakeCanBusStatus(CanIndex) + 0x01;
     Device[addr+2].Buf[0] =  Can.Buf[6];
     Device[addr+2].Buf[1] =  Can.Buf[7] & 0x07;
     Device[addr+2].Buf[2] = 0;
}

// 四个字节一组数据  数据低八位 数据高八位 CRC 名称
void SwitcherDataDeal(u32 Id,u8 *buf,u8 CanIndex)
{
	u8 i,j,fram;
	u8 status;
	u8 SwitcherAddr,SensorAddr;
	u8 Crc,State,Name;
	u16 Data;
	
	fram = (Id >> 24) & 0x00000000F; //获取ID中的帧序号
	SwitcherAddr = Id & 0xFF; //转换板地址
	i = DeviceManger.GetSwitcher(SwitcherAddr);
	if(Switchers[i].Addr == 0xFF)
		return;
	Switchers[i].Addr = SwitcherAddr;
	Switchers[i].Tick = SYS_TICK;
	Switchers[i].Flag |= 0x80;//转换板已启用标记
	
	SensorAddr = SwitcherAddr + fram *2 + 1;//转换板每帧数据上传两个传感器信息
	
	for(i=0;i<2;i++)
	{
		j = 4*i;
		Data = buf[j+1];
		Data <<= 8;
		Data |= buf[j];
		Crc = buf[j+2];
		State = ((buf[j+3] & 0xC0) >> 6);
		Name = (buf[j+3] & 0x3F);
		
		if((Name == 0x3F) || (!(Device[SensorAddr - 1].Flag & 0x01)))
          {
			Device[SensorAddr - 1].Name = Name;
               Device[SensorAddr-1].Flag |= 0x02;
               SensorAddr++;
			continue;
          }
		
		if(State & 0x02)//转换板上报该传感器掉线
		{
			Device[SensorAddr - 1].Status |= 0x80;
			Device[SensorAddr - 1].Flag &= ~0x02;
			Device[SensorAddr - 1].Buf[0] = 0;
			Device[SensorAddr - 1].Buf[1] = 0;
		}
		else
		{
			Device[SensorAddr - 1].Status &= ~0x80;
			Device[SensorAddr - 1].Tick = SYS_TickSec;
			Device[SensorAddr - 1].Name = Name;
			status = 0;
			if(State & 0x01) //传感器为开关量传感器或断电器
			{
				status |= MakeCanBusStatus(CanIndex); //Can编号
				Device[SensorAddr - 1].Status = status;
				if(Device[SensorAddr - 1].Name == 0x1F)
					CheckCrc(SensorAddr,Crc,1);//断电器校验初始化CRC
				else
					CheckCrc(SensorAddr,Crc,0);//开关量传感器校验初始化CRC
				Device[SensorAddr - 1].Status |= (Data & 0x03); //开关量传感器监测值
			}
			else//模拟量传感
			{
				status |= ((Data >> 11) & 0x03);//模拟量监测值数据类型     
				status <<= 2;
				status |= ((Data >> 13) & 0x03);//模拟量监测值数据小数位数  
				status &= 0x0F;
				status |= MakeCanBusStatus(CanIndex); //Can编号
				status &= ~0x10;
				Device[SensorAddr - 1].Status = status;
				Device[SensorAddr - 1].Buf[0] = Data;
				Device[SensorAddr - 1].Buf[1] = ((Data >> 8) & 0x07);
				CheckCrc(SensorAddr,Crc,1);
			}
		}
		SensorAddr++;
	}
}

//Can数据区8个字节，每个字节代表两个传感器联动信息，
//4个比特位代表1个传感器信息，D3、D2：00--模拟量传感器 10---开关量 11---断电器
//D1：0---传感器在线  1---传感器掉线
//D0：0---模拟量为该传感器未超限，开关量表示当前为0态   1---模拟量为该传感器已超限，开关量表示当前为1态
void SwitherCtrInfoDeal(u32 Id,u8 *buf)
{
	u8 i,j,SwitcherAddr,SensorAddr,SensorInfo1;
	u8 SensorInfo;
	
	SwitcherAddr = Id & 0xFF; //转换板地址
	i = DeviceManger.GetSwitcher(SwitcherAddr);
	if(Switchers[i].Addr == 0xFF)//转换板数量已达上限，新转换板数据不处理
		return;
	
	Switchers[i].Addr = SwitcherAddr;
	Switchers[i].Tick = SYS_TICK;
	Switchers[i].Flag |= 0x80;//转换板已启用标记
	
	for(i=0;i<8;i++)
	{
		//每个字节两个传感器信息
		SensorInfo1 = buf[i];
		for(j=0;j<2;j++)
		{
			SensorAddr = SwitcherAddr + 2*i + j + 1;
			if(!(Device[SensorAddr-1].Flag & 0x01))//上位机未定义，不处理该传感器信息
				continue;
			//取出传感器信息4bit   传感器类型2bit + 在线/离线状态1bit + 数值超限1bit
			if(j==0)
				SensorInfo = ((SensorInfo1 >> 4) & 0x0F);
			else
				SensorInfo = (SensorInfo1 & 0x0F);
               
       //传感器类型 00:模拟量传感器  10:开关量传感器    0x03:断电器
			switch(SensorInfo & 0x0C)
               {
               case 0x0C:              // 断电器
                    Device[SensorAddr - 1].Status &= ~0x03;
				Device[SensorAddr - 1].Status |= (SensorInfo & 0x03);
				break;
               case 0x00:              // 模拟量传感器
               case 0x08:              // 开关量传感器
                    if(SensorInfo & 0x02)         //传感器断线
                    {
                         Device[SensorAddr - 1].Status |= 0x80;
                         CheckLianDongInfo(SensorAddr,0x10,0x10);
                    }
                    else                         // 传感器在线
                    {
                         if((SensorInfo & 0x0C) == 0x08)      // 开关量
                         {
                              CheckLianDongInfo(SensorAddr,0x20,(SensorInfo & 0x01));
                         }
                         if((SensorInfo & 0x0C) == 0x00)      // 模拟量
                         {
                              if(SensorInfo & 0x01)//模拟量传感器超限
                                   CheckLianDongInfo(SensorAddr,0x01,0x01);
                              else//未超限
                                   CheckLianDongInfo(SensorAddr,0x01,0x00);
                         }
                    }
                    break;
               }
		}
	}
}

void HandleCanData(u32 ID,u8 CanIndex)
{
	u8 addr,cmd,status,k;
	status = 0;
	addr = ID;
	
	Can.Tick[CanIndex-1] = SYS_TICK;
	SendDebugData(ID,Can.Buf,Can.Len,0);

	cmd = ((ID >> 17) & 0x7F);

	if(cmd == 0x55)
	{
		SwitcherDataDeal(ID,&Can.Buf[0],CanIndex);
		return;
	}
	
	if(cmd == 0x56)
	{
		SwitherCtrInfoDeal(ID,&Can.Buf[0]);
		return;
	}
	
	if(cmd == 0x57)
	{
		SendBreak3_0();
		return;
	}
	
	if(addr == 0)
		return;
	
	if((cmd != 0x0B) && (cmd != 0x0C) && (cmd != 0x55) && (cmd != 0x56) && (cmd != 0x57))                      // 0B 0C ??????????
	{
		Device[addr-1].Name = ((ID >> 8) & 0x3F);      //????
		Device[addr-1].Tick = SYS_TickSec;
		Device[addr-1].Status &= ~0x80;
	}
	
	if(!(Device[addr-1].Flag & 0x01))    // ?????????????
	{
		Device[addr-1].Flag |= 0x02;    // ?????????
		CanSendData(CanIndex,MakeFramID(0,UnRegister,addr),&Can.Buf[0],0);
		if(cmd < 0x10)                 // ??????,???,????????,????
			return;
	}
	
     if(Ack.NeedToNet == 1)       //??????
     {
          if(MsTickDiff(Ack.Tick) > 1000)    // 1S ??
               Ack.NeedToNet = 0;
     }
     switch(cmd)        //
     {
     case 0x01:        //模拟量传感器定时上报数据
		if(ID & 0x01000000)
			CheckLianDongInfo(addr,0x01,0x01);
		else
			CheckLianDongInfo(addr,0x01,0x00);
		if(CheckTime(&Can.Buf[2]) == 1)
		{
			SyncTime(CanIndex,addr);                  
		}
		MakeSensorCanData(ID,CanIndex,addr);    
		CheckCrc(addr,Can.Buf[6],1);
		break;
		
     case 0x02:
		AckFunc(CanIndex,ACK,addr);
          if((Can.Buf[0] == 0xFF) && (Can.Buf[1] == 0xFF))  //????
          {
               WriteSensorRecord(addr,DEVICERESET);
               break;
          }
          WriteSensorRecord(addr,SENSORVARY);
          break;
     case 0x03:
		status += MakeCanBusStatus(CanIndex);
          Device[addr-1].Status &= 0x0E;   // ?? D6 D5 D0
		Device[addr-1].Status += status;
		if(Device[addr-1].Name == 0x21)  //?????,????
		{
	          Device[addr-1].Buf[0] = Can.Buf[0];
			Device[addr-1].Buf[1] = Can.Buf[1];
               AckFunc(CanIndex,ACK,addr);
		}
	     else // ??????,??0??1  2?????,0000??0  0001??1
		{  
               if(ID & 0x01000000)      // ??????????????
               {
                    if(Device[addr-1].Crc & 0x08)             // 1???
                    {
                         if(Can.Buf[1] & 0x01)
                              CheckLianDongInfo(addr,0x40,0x40);
                    }
                    if(Device[addr-1].Crc & 0x04)             // 0???
                    {
                         if(!(Can.Buf[1] & 0x01))
                              CheckLianDongInfo(addr,0x20,0x20);
                    }
               }
               else
               {
                    if(Device[addr-1].Crc & 0x08)            // 1???
                         CheckLianDongInfo(addr,0x40,0);
                    if(Device[addr-1].Crc & 0x04)             // 0???
                         CheckLianDongInfo(addr,0x20,0);
               }
			Device[addr-1].Status += (Can.Buf[1] & 0x01);   //???+??
			Device[addr-1].Buf[0] = 0;
			Device[addr-1].Buf[1] = 0;
               AckFunc(CanIndex,ACK,addr);
               CheckCrc(addr,Can.Buf[2],0);
		}
          break;
     case 0x04:                 //???????????
          if(CheckTime(&Can.Buf[0]) == 1)
          {
               SyncTime(CanIndex,addr);
               return;
          }
          else
               AckFunc(CanIndex,ACK,addr);
          Device[addr-1].Status = MakeCanBusStatus(CanIndex);        //???
          if(Device[addr-1].Warn)
               Device[addr-1].Status |= 0x02;
          Device[addr-1].Buf[0] = 0;    
          Device[addr-1].Buf[1] = 0;
          Device[addr-1].Buf[2] = 0;
          CheckCrc(addr,Can.Buf[4],1);
          break;
     case 0x05:               //读卡器上传卡片数据， 卡片编号低  高  RSSI  时间域
          AckFunc(CanIndex,ACK,addr);
          if((Can.Buf[0] == 0xFF) && (Can.Buf[1] == 0xFF))  //????
          {
               WriteSensorRecord(addr,DEVICERESET);
               Device[addr-1].Buf[0] = 0x00;        //???
               break;
          }
          Device[addr-1].Status &= 0x0F;
          Device[addr-1].Status += MakeCanBusStatus(CanIndex);        //???
          if(Can.Len == 7)
               WriteSensorRecord(addr,CARDVARY);
          if(Can.Len == 8)
               WriteSensorRecord(addr,NEWCARDVARY);
          break;
     case 0x06://电源设备信息上传,根据1---8帧来填充数据
          AckFunc(CanIndex,ACK,addr);
          Device[addr-1].Status &= 0x0E;
          Device[addr-1].Status += (MakeCanBusStatus(CanIndex) + (Can.Buf[0] & 0x01)); //状态位   交直流              
          Device[addr-1].Buf[0] = 0;
          Device[addr-1].Buf[1] = 0;
          Device[addr-1].Buf[2] = 0;
          WritePowerRecord(addr,&Can.Buf[0]);
          break;
     case 0x07:                                       // 应急广播上传状态
          status += MakeCanBusStatus(CanIndex);
          Device[addr-1].Status &= 0x0C;
          Device[addr-1].Status += (status + (Can.Buf[0] >> 7));
          if(Device[addr-1].Warn & 0x7F)
               Device[addr-1].Status |= 0x02;
          Device[addr-1].Buf[0] = Can.Buf[0] & 0x7F;        
          Device[addr-1].Buf[1] = 0;
          AckFunc(CanIndex,ACK,addr);
          CheckCrc(addr,Can.Buf[5],1);
          break;
     case 0x08:
          break;
	 
     case 0x09:
          if(CheckTime(&Can.Buf[2]) == 1)
               SyncTime(CanIndex,addr);
          else
               AckFunc(CanIndex,ACK,addr);
          if((Can.Buf[0] == 0xFF) && (Can.Buf[1] == 0xFF))  //????
          {
               WriteSensorRecord(addr,DEVICERESET);
               break;
          }
          // buf[0]????  buf[1]??????????
          status += MakeCanBusStatus(CanIndex);
          Device[addr-1].Status &= 0x80;        //D4???????
          if((Device[addr-1].Warn & 0x7F) && (!(Can.Buf[0] & 0x0C)))        //????,??????
               Device[addr-1].Status += (status + (Can.Buf[0] & 0x1E) + 0x01);
          else                                  //????,??????
               Device[addr-1].Status += (status + (Can.Buf[0] & 0x1F));   //D7 D6 D5 CAN??????
          Device[addr-1].Buf[0] =  Can.Buf[0];   //???????
          Device[addr-1].Buf[1] =  Can.Buf[1];   //??????
#ifndef FengDianNet
          CheckCrc(addr,Can.Buf[6],1);
#endif
          break; 
     case 0x0A:                //断电器联动信息
          //AckFunc(CanIndex,ACK,addr);
          WriteSensorRecord(addr,LianDongRecord);
          break;
     case 0x0B:               // MAS??????????
//          AckFunc(CanIndex,ACK,addr);
//          MakeMASWirelessSensor(CanIndex,addr);
          break;
	case 0x0D: //综保信息上传
		//AckFunc(CanIndex, ACK, addr);
		Device[addr - 1].Status = 0;
		Device[addr - 1].Status += MakeCanBusStatus(CanIndex); //状态位            
//		Device[addr - 1].Buf[0] = 0;
		Device[addr - 1].Buf[1] = 0;
		Device[addr - 1].Buf[2] = 0;
		DeviceManger.WriteRecord_IPD_ZJM(addr, &Can.Buf[0], ID,Can.Len);
		break;
	case 0x0E://北京院电源上传数据
		//AckFunc(CanIndex, ACK, addr);
		k = Can.Buf[0];
		k = ~k;
		k &= 0x01;
		Can.Buf[0] &= 0xFE;
		Can.Buf[0] |= k;
		Device[addr - 1].Status = 0;
		Device[addr - 1].Status += (MakeCanBusStatus(CanIndex) + (Can.Buf[0] & 0x01)); //状态位   交直流              
		Device[addr - 1].Buf[0] = 0;
		Device[addr - 1].Buf[1] = 0;
		Device[addr - 1].Buf[2] = 0;
		DeviceManger.WriteRecord_PekingPower(addr,Can.Buf,Can.Len);
		break;
     case 0x0F:                  // ????????
          MakeSensorCanData(ID,CanIndex,addr);
          break;
     case 0x10:
          if((Ack.NeedToNet == 1) && (Ack.Addr == addr))
          {
               CanInfoToNet(ACK,&Can.Buf[0],0);
               Ack.NeedToNet = 0;
               break;
          }
          break;
     case 0x39:         // 读卡器/广播对联动的回应
          Device[addr-1].Warn &= 0x7F;
          LianDong.Retry = 0;
          break;
     
     case 0x51:      //????????
          Init.Retry = 0;
          if((Init.FramStatus == F_Single) || (Init.FramStatus == F_End))
          {
               Init.Status = INITIDLE;     //?????
               Init.W = 0;
               break;
          }
          Init.R += Init.Shift;
          if(Init.R + 8 < Init.W)
               Init.FramStatus = F_Middle;
          else
               Init.FramStatus = F_End;
          Init.FramCnt ++;
          if(Init.FramCnt > 3)
               Init.FramCnt = 0;
          Init.Status = SENDPREPARE; 
          break;
     default:                   // ????,????????
          CanInfoToNet(cmd,&Can.Buf[0],Can.Len);
          break;
     }
}

// ??11?????????
// ??????  ?????1???2???????4??   SUM
// ??t??????,flag??????
void WriteSensorRecord(u8 addr,u8 flag)
{
     u8 buf[11],D5Bit,i;
     if(Sys.Flag & 0x02)    // ??????,?D5?1
          D5Bit = 0x20;
     else
          D5Bit = 0;
     switch(flag)
     {
     case DEVICERESET:
          buf[0] = addr;   //??
          buf[1] = Device[addr-1].Name;    //??
          buf[2] = 0x40 + D5Bit;    //???????
          buf[3] = Can.Buf[2];
          buf[4] = 0;
          buf[5] = 0;

          buf[6] = Can.Buf[3];      //????
          buf[7] = Can.Buf[4];
          buf[8] = Can.Buf[5];
          buf[9] = Can.Buf[6];
          break;
     case SENSORVARY:
          buf[0] = addr;    //??
          buf[1] = Device[addr-1].Name;   //??
          buf[2] = (Device[addr-1].Status & 0x0F) + D5Bit;  //????--??????
          buf[3] = Can.Buf[0];//Device[addr-1].Buf[0];  //??
          buf[4] = Can.Buf[1] & 0x07;//Device[addr-1].Buf[1];
          buf[5] = 0x00;//Device[addr-1].Buf[2];

          buf[6] = Can.Buf[3];
          buf[7] = Can.Buf[4];
          buf[8] = Can.Buf[5];
          buf[9] = Can.Buf[6];
          break;

     case CARDVARY:
          buf[0] = addr;    //??
          buf[1] = Device[addr-1].Name;   //??
          buf[2] = (Can.Buf[1] >> 4) + D5Bit;  //????--?????????
          buf[3] = Can.Buf[0];
          buf[4] = (Can.Buf[1] & 0x0F);
          buf[5] = Can.Buf[2];    //RSSI 

          buf[6] = Can.Buf[3];
          buf[7] = Can.Buf[4];
          buf[8] = Can.Buf[5];
          buf[9] = Can.Buf[6];
          break;
     case NEWCARDVARY:
          buf[0] = addr;
          buf[1] = Device[addr-1].Name;
          buf[2] = Can.Buf[3] + D5Bit;
          buf[3] = Can.Buf[0];
          buf[4] = (Can.Buf[1] & 0x3F);
          buf[5] = Can.Buf[2];    //RSSI 

          buf[6] = Can.Buf[4];
          buf[7] = Can.Buf[5];
          buf[8] = Can.Buf[6];
          buf[9] = Can.Buf[7];
          break;
     
     case NETRESET:           //??????
          buf[0] = Sys.Addr;       //??
          buf[1] = NetAttri;      //??
          buf[2] = 0x40 + D5Bit;          //??
          buf[3] = Sys.ResetTimes;  //??
          buf[4] = 0;
          buf[5] = 0;
          TimeChange(&buf[6]);   //buf[6,7,8,9]????
          break;        
     case LianDongRecord:
          buf[0] = addr;    //??
          buf[1] = Device[addr-1].Name;   //??
          buf[2] = 0x10 + D5Bit;         //???????
          for(i = 0;i < 7;i ++)
               buf[3+i] = Can.Buf[i];
          break;         
     }
     buf[10] = SumAll(buf,10);
     FramWriteBurstByte(Sys.W,buf,11);
     Sys.W += 11;
     if(Sys.W >= FramEnd)
          Sys.W = FramBegin;
     UpdateWptr();
}

/*
 ??8???:???0----7  ??7???7???,????5???
 * ?????????????+?????
 */
void WritePowerRecord(u8 addr,u8 *buf)
{
     u8 i,j,t,fram;
     if(addr == 0)
          return;
     i = FilterPowerAddr(addr);
     if(i == 0xFF)
          return;
     Power[i].Addr = addr;
     fram = buf[0] >> 1;
     if(fram == 7)
          t = 4;
     else 
          t = 6;
     for(j = 0;j < t;j ++)
     {
          Power[i].BatInfo[fram*6 + j] = buf[1+j];
     }
}
// ????
void SyncTime(u8 CanIndex,u8 addr)
{
     u8 buf[6];
     Can.ID = (u32)MakeFramID(0,0x23,addr);
     buf[0] = Time.Hex[0];       // ?
     buf[1] = (Time.Hex[3] << 5) +Time.Hex[1];       // ??date??
     buf[2] = Time.Hex[2];
     buf[3] = Time.Hex[4];
     buf[4] = Time.Hex[5];
     buf[5] = Time.Hex[6];      // ?
     CanSendData(CanIndex,Can.ID,buf,6);
	 SendDebugData(Can.ID,buf,6,3);
}

u16 ShiftTimeDiff(u16 t)
{
     if(Time.AllSecond >= t)
          return Time.AllSecond - t;
     else
          return 3600 - t + SYS_TICK;
}


u8 CheckTime(u8 *buf)
{
     u16 t;
     u8 min,sec;
     if(Sys.SyncLeft)
          return 0;
     t = buf[1];
     t <<= 8;
     t += buf[0];
     t &= 0x0FFF;
     min = t >> 6;
     sec = t & 0x3F;
     t = (min * 60) + sec;   // ??????5s
     if(t <= Time.AllSecond)
     {
          if(((Time.AllSecond - t) < TimeShift) || ((t + 3600 - Time.AllSecond) < TimeShift))
               return 0;
     }
     else
     {
          if(((t - Time.AllSecond) < TimeShift) || ((3600 + Time.AllSecond - t) < TimeShift))
               return 0;
     }
     return 1; 
}

