#include "NetPro.h"
#include "Public.h"
#include "W5500.h"
#include "sys.h"
#include "cpu.h"
#include "VoicePro.h"
#include "config.h"
#include "Udp.h"
#include "FmInit.h"
#include "CanPro.h"
#include "SD2068.h"
#include "DeviceManger.h"

extern _DeviceManger DeviceManger;
extern CSys Sys;
extern CVoice Voice;
CPower Power[MaxPowerNum];
extern CCan Can;
extern CNet Net;
extern vu16 SYS_TICK,SYS_TickSec;
extern u16 LedRunTime;
extern CSys Sys;
extern CTime Time;
extern u8 ConfigBuf[10];
extern CSensor Device[MaxDeviceNum];
extern CSendInit Init;
extern CExcute Excute[MaxTriggerNum];

void NetIntProcess(void)
{
     u8 flag;
IntDispose:
     flag = ReadReg1Byte(IR);
     if(flag)
     {
          WriteReg1Byte(IR,(flag & 0xf0));
          if((flag & CONFLICT) == CONFLICT) 
               Net.Status = NET_IPCONFLICT;
          if((flag & UNREACH) == UNREACH)
               Net.Status = NET_UNREACH;
     }
     flag = ReadReg1Byte(SIR);
     switch(flag & 0x07)
     {
          case 0x00:return;
          case 0x01:Net.Socket = 0;break;
          case 0x02:Net.Socket = 1;break;
          case 0x03:Net.Socket = 0;break;
          case 0x04:Net.Socket = 2;break;
          case 0x05:Net.Socket = 0;break;
          case 0x06:Net.Socket = 1;break;
          case 0x07:Net.Socket = 0;break;
     }
     flag = ReadSocket1Byte(Net.Socket,Sn_IR);
     WriteSocket1Byte(Net.Socket,Sn_IR,flag);
     if(flag & IR_CON) 
     {
          Net.Status = NET_CONNECT;
          Net.Tick[Net.Socket] = SYS_TICK;
     }
     if((flag & IR_DISCON) || (flag & IR_TIMEOUT))
     {
          if(Net.Socket != 3)
          {
               WriteSocket1Byte(Net.Socket,Sn_CR,CLOSENET);
               Socket_Lisen(Net.Socket);
			if(Net.Socket == Sys.DebugSocket)
				Sys.DebugSocket = 0xFF;			
          }
     }
     if(flag & IR_RECV)
     {
          Net.Tick[Net.Socket] = SYS_TICK;
          Net.Len = ReadSocketData(Net.Socket,&Net.Buf[0]);
          if(Net.Socket < 3)
          {
               Sys.Flag &= ~((0x08 << Net.Socket) + 0x02); 
               HandleTcpData(&Net.Buf[0],Net.Len,0);
          }
          else
          {
               Sys.Flag &= ~0x20;
               Net.UdpDipr[0] = Net.Buf[0];
               Net.UdpDipr[1] = Net.Buf[1];
               Net.UdpDipr[2] = Net.Buf[2];
               Net.UdpDipr[3] = Net.Buf[3];
               Net.UdpPort = Net.Buf[4];
               Net.UdpPort <<= 8;
               Net.UdpPort += Net.Buf[5];
               HandleTcpData(&Net.Buf[8],Net.Len-8,1);
          }
     }
     if(flag & IR_SEND_OK)
          Net.Status = NET_IDLE;
     if(ReadReg1Byte(SIR) != 0)
          goto IntDispose;
}


void NetPro(void)
{
     u8 i;
     switch(Net.Status)
     {
     case NET_INIT:
          LedRunTime = 50;
          W5500Init();
          Net.Tick[0] = SYS_TICK;
          Net.Status = NET_LINK;
          break;
     case NET_LINK:
          if((ReadReg1Byte(PHYCFGR)&LINK) !=0)
          {
               LoadNetParameters();
               DetectGateway();
               Net.Status = NET_MODE;
               LedRunTime = 500;
          }
          else
          {
               if(MsTickDiff(Net.Tick[0]) > 5000)
                    Net.Status = NET_INIT;
          }
          break;
     case NET_MODE:
          Socket_Lisen(0);
          Socket_Lisen(1);
          Socket_Lisen(2);
          Socket_Udp(3);
          Net.Tick[0] = SYS_TICK;
          Net.Tick[1] = SYS_TICK;
          Net.Status = NET_IDLE;
          break;
     case NET_IDLE:
     case NET_CONNECT:
     case NET_INT:
          NetIntProcess();
          for(i = 0;i < 3;i ++)
          {
               if(MsTickDiff(Net.Tick[i]) > 15000)   //15S
               {
                    Net.Tick[i] = SYS_TICK;
                    Socket_Lisen(i);
                    Sys.Flag |= (0x08 << i);
               }
          }
          if((Sys.Flag & 0x18) == 0x18)   //socket0 socket1
          {
               Sys.Flag |= 0x02;
          }
          break;
     default:
          Net.Status = NET_INIT;
          break;
     }
}


u8 CheckRecBuf(u8 *buf,u16 len)
{
     u16 t;
     if(len < 10)
          return 0;
     if((buf[0] != 0xFE) || (buf[1] != 0xFE) || (buf[2] != 0xFE) ||(buf[3] != 0xFE))
     {
          AnswerWrongAck();
          return 0;
     }
     t = buf[7];
     t <<= 8;
     t += buf[6];
     if(t != len)
          return 0;
     if(CRC16Check(buf,len) == 0)
          return 0;
     return 1;
}

// FLAG = 0 处理 TCP数据包   FLAG = 1 处理 UDP 数据包
// 认为当数据个数小于30个时，以命令来处理。大于30个时，认为是语音数据
void HandleTcpData(u8 *buf,u16 len,u8 flag)
{
	u16 i,j;
	if((buf[0]==0x55) && (buf[1] == 0xAA) && (buf[2] == 0x55) && (buf[3] == 0xAA)) // 语音数据 
	{
		if(Sys.PauseVoice)
		{
			AnswerCycleSec(); 
			Voice.W = Voice.R;
			return;
		}
			
		i = buf[5];
		j = buf[6];
		Net.VoiceCnt = i + j * 256;  
		for(i = 0;i < len;i += 507)
			{
			Voice.Dir = PcToDevice;
			Voice.DestAddr = buf[i+4];
			for(j = (i+7);j < (507+i);j ++)
			{
				Voice.RecBuf[Voice.W++] = buf[j];
				if(Voice.W == MaxVoiceLen)
				Voice.W  = 0;
			}
			}
			AnswerCycleSec();    
     }
     else              // 命令
     {
		HandleCmd(buf,len,flag);
     }
}

// FLAG = 0 TCP 命令  FLAG=1 UDP 数据
void HandleCmd(u8 *buf,u16 len,u8 flag)
{
     u16 t;
     if(CheckRecBuf(buf,len) == 0)
          return;
     switch(buf[9])
     {
     case 0x60:
          if(Sys.SyncLeft)
               Sys.SyncLeft --;
          if(Sys.InitLeft)
               Sys.InitLeft --;
          if(Sys.Delay)
          {
               AnswerAck(1);          
               break;
          }
          if(flag == 1)        // 时间
          {
               ;
          }
          if(buf[len-3] == 0x00)
          {
               if(Sys.R != Sys.W)
               {
                    if(Sys.R + Sys.Shift < FramEnd)
                         Sys.R += Sys.Shift;
                    else
                         Sys.R = Sys.Shift + Sys.R - FramEnd + FramBegin;
                    UpdateRptr();
                    Sys.Flag &= ~0x01;
                    Sys.Shift = 0;
               }
          }
          AnswerCurInfo(flag);
          break;
     case 0x62:AnswerUnRegisterInfo();break;              // ???????
     case 0x64:AnswerCheckCrcCntInfo();break;             // ?????????
     case 0x65:AnswerCleanCrcCntInfo();break;             // ???????
     case 0x66:AnswerCrcFail();break;                     // ??????????
     case 0x67:AnswerResetCrcFail();break;                // ????????????
     case 0x68:AnswerCrcInfo();break;                     // ?????????crc
     case 0x70:AnswerIpAddr();break;                      //??IP??,???
     case 0x71:AnswerBasePara();break;                    //????????,???????????????????
     case 0x72:AnswerSoftVerb();break;                    // ?????????????
     case 0x80:AnswerSetIp();break;                       //????IP??,???,????
     case 0x81:AnswerSetResetTimes();break;               //??????
     case 0x82:AnswerSetTime();break;                     //????
     case 0x83:AnswerFactoryTime();break;                 //??????
     case 0x84:AnswerCleanRWptr();break;                  //?????????
     case 0x87:AnswerSetMac();break;                       // ??MAC??
     case 0x88:AnswerCheckMac();break;                    // ??MAC??
     case 0x89:AnswerSetNetFilterTime();break;            // ?????????
     case 0x8E:AnswerCleanRecTimes();break;             // ???????????
     case 0x8F:AnswerCleanNetCrc();break;             // ????Crc
	 case 0x90:AnswerSet3_0Break();break;					//上位机配置瓦斯3.0断电
     case 0x8A:AnswerCheckNetFilterTime();break;           // ?????????
     case 0x8B:AnswerSetBiaoJiao();break;                  // ????????,buf[12] ?? buf[13] ??
     case 0x8C:AnswerReseetNet();break;                   //????
     case 0x8D:AnawerSetUdp();break;                      // ??UDP
     case 0x94:AnswerPowerInfo(buf[12]);break;            //??????
	 case 0xEA:AnswerExcuteList();break;
	 case 0xEC:AnswerSetDebugSocket(buf[12]);break;
	 case 0xED://查询北京院电源信息
		AnswerGetPkPowerInfo(buf[12]);
		break;
     case 0xEE:AnswerForBootLoader();break;                 // ??bootloader,?????????IP????
	
	 case 0xEF://查询综保信息
		AnswerGetIPDInfo(buf[12]);
		break; 
	
	 case 0x63:   //
          t = buf[11];   //
          t <<= 8;
          t += buf[10];
          t -= 3;
          AnswerErrorBitsInfo(&buf[12],t);
          break;
     case 0x85:  //发送初始化信息
          Sys.InitLeft = 5;
          Sys.CrcCnt ++;
          WriteConfig(NetCrcCntAddr,1);
          t = buf[11];
          t <<= 8;
          t += buf[10];
          t -= 3;
          IWDG_Feed();
          ConfigBuf[0] = t;
          ConfigBuf[1] = t >> 8;
          WriteConfig(InitLenAddr,2);
          FramWriteBurstByte(BaseInfoAddr,&buf[12],t); 
          t = CalCrc16(&buf[12],t,0);
          ConfigBuf[0] = t;
          ConfigBuf[1] = t >> 8;
          WriteConfig(InitCrcAddr,2);
          AnswerAck(0);
          ReadDefaultInit();
          break;
     case 0x86:
          Sys.Addr = buf[12];
          ConfigBuf[0] = Sys.Addr;
          WriteConfig(NetAddrAddr,1);
          AnswerAck(0);
          break;
     case 0x69:         // 心跳指令
          Sys.VoiceSocket = Net.Socket;
          AnswerCycleSec();
          break;
     default:
          if((buf[10] >=3))
               NetInfoToCanOrRs485(buf[9],&buf[12],buf[10]-3);
          else
               AnswerWrongAck();
          break;  
     }
}

u16 CalPtrLen(void)
{
     if(Sys.W >= Sys.R)
     {
          if(Sys.W - Sys.R >= MaxOneFrame)
               Sys.Shift = MaxOneFrame;
          else
               Sys.Shift = Sys.W - Sys.R;
     }
     else
     {
          if(FramEnd - Sys.R + Sys.W - FramBegin >= MaxOneFrame)
               Sys.Shift = MaxOneFrame;
          else
               Sys.Shift = FramEnd - Sys.R + Sys.W - FramBegin;
     }
     return Sys.Shift;
}

void MakeFrameHeader(void)
{
     Net.Buf[0] = 0xEF;
     Net.Buf[1] = 0xEF;
     Net.Buf[2] = 0xEF;
     Net.Buf[3] = 0xEF;
     Net.Buf[4] = 0x00;    //????
     Net.Buf[5] = Sys.Addr;
     Net.Buf[6] = 0x00;    // ????
     Net.Buf[7] = 0x00;
}

// flag = 0 TCP   FLAG = 1 UDP
void AnswerCurInfo(u8 flag)
{
	u16 i,j,k,l,crc,t;

	u8 IPDCurInfo[MaxIPDNum][20],n,IPDCnt=0,proCnt=0;;
	MakeFrameHeader();
	t = 9;
	proCnt++;
	Net.Buf[t++] = 0x60;        //????  
	Net.Buf[t++] = 0;          //???????  ?? + ??2?? + ????8?? + N*5
	Net.Buf[t++] = 0;           //???????, ????

	Net.Buf[t++] = Sys.R;        //???
	Net.Buf[t++] = Sys.R >> 8;
	Net.Buf[t++] = Sys.W;        //???
	Net.Buf[t++] = Sys.W >> 8;

	TimeChange(&Net.Buf[t]);
	t += 4;
	j = 0;
	for(i = 0;i < MaxDeviceNum;i ++)
	{
		if(flag == 1)       //如果是UDP，则只传输通讯正常的设备
		{
			if((!(Device[i].Status & 0x80))  && (Device[i].Flag & 0x01))
			{
				Net.Buf[t++] = i + 1;                //????
				Net.Buf[t++] = Device[i].Name;       //??
				Net.Buf[t++] = Device[i].Status;    //??
				Net.Buf[t++] = Device[i].Buf[0];    //?????
				Net.Buf[t++] = Device[i].Buf[1];    //?????
				if(Device[i].Name == 0x38)
				{
					DeviceManger.ReadRecord_IPD_ZJM(IPDCurInfo[IPDCnt],i+1);
					IPDCnt++;
				}
                    j ++;
			}
		}
		else             //如果是TCP通讯，则传输全部设备信息
		{
			if(Device[i].Flag & 0x01)    //上位机定义了该设备
			{
				Net.Buf[t++] = i + 1;
				Net.Buf[t++] = Device[i].Name;
				Net.Buf[t++] = (Device[i].Status & (~0x10));
				Net.Buf[t++] = Device[i].Buf[0];
				Net.Buf[t++] = Device[i].Buf[1];
				if((!(Device[i].Status & 0x80)) && (Device[i].Name == 0x38))
				{
					DeviceManger.ReadRecord_IPD_ZJM(IPDCurInfo[IPDCnt],i+1);
					IPDCnt++;
				}
				j ++; 
			}
		}
	}
	j = j*5 + 11;
	Net.Buf[10] = j;
	Net.Buf[11] = j >> 8;
     
	if(Sys.R != Sys.W)
	{
		CalPtrLen();
		proCnt++;
		Net.Buf[t++] = 0x61;                    //CMD
		k = t;                         //????????
		Net.Buf[t++] = 0;           //??????
		Net.Buf[t++] = 0;
		j = Sys.R;                      // ?????
		l = 0;
		for(i = 0;i < Sys.Shift;)
		{
			if(ReadRecord(j,&Net.Buf[t]))
			{
				t += 10;
				l += 10;            //????
			}
			i += 11;
			j += 11;
			if(j >= FramEnd)
				j = FramBegin;
		}
		Net.Buf[k] = l+3;
		Net.Buf[k+1] = (l+3)>>8;
	}
	if(IPDCnt != 0)
	{
		proCnt++;
		Net.Buf[t++] = 0x64;         //CMD
		k = t;
		Net.Buf[t++] = 0;           //??????
		Net.Buf[t++] = 0;
		l = 0;
		for(n=0;n<IPDCnt;n++)
		{
			BufCopy(&Net.Buf[t],IPDCurInfo[n],20);
			t += 20;
			l += 20;
		}
		Net.Buf[k] = l+3;
		Net.Buf[k+1] = (l+3) >> 8;
	}
	Net.Buf[8] = proCnt;
	Net.Buf[t++] = Sys.Crc;
	Net.Buf[t++] = Sys.Crc >> 8;

	Net.Buf[6] = t + 2;    //?????????
	Net.Buf[7] = (t+2) >> 8;
     
	crc = CalCrc16(&Net.Buf[0],t,0);
	Net.Buf[t++] = crc;
	Net.Buf[t++] = crc >> 8; 
	SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerCleanRecTimes(void)
{
     u8 i;
     for(i = 0;i < MaxDeviceNum;i ++)
          Device[i].RecTimes = 0;
     AnswerAck(0);
}

void AnswerCleanNetCrc(void)
{
     u8 i;
     for(i = 0;i < MaxDeviceNum;i ++)         // ?????,?????????????
          Device[i].Name = 0xFF;
     Sys.Crc = 0;
     AnswerAck(0);
}

void AnswerSet3_0Break(void)
{
	u8 i,j;
	u16 DataLen;
	j=0;
	Breaks3_0.Cnt = 0;
	for(i=0;i<8;i++)
	{
		Breaks3_0.CH4Addr[i] = 0;
		Breaks3_0.BreakerAddr[i] = 0;
	}
	DataLen = Net.Buf[11];
	DataLen <<= 8;
	DataLen += Net.Buf[10];//数据域长度
	DataLen -=4;
	i = DataLen >> 1;
	if(i>4)
		i=4;
	do
	{
		Breaks3_0.CH4Addr[j] = Net.Buf[13+2*j];
		Breaks3_0.BreakerAddr[j] = Net.Buf[13+2*j+1];
		Breaks3_0.Cnt++;
		j++;
	}while(--i);
	SendBreak3_0();
	AnswerAck(0);
}

void AnswerGetPkPowerInfo(u8 addr)
{
	u16 k,crc,t;
	t = 8;
	k = 3;
	MakeFrameHeader();
	Net.Buf[t++] = 0x01;
	Net.Buf[t++] = 0xED; //协议编码
	Net.Buf[t++] = 0; //编码长度低八位  编码 + 长度2字节 + 网关信息8字节 + N*5
	Net.Buf[t++] = 0; //编码长度高八位, 最后更新
	if(addr == 0x00)//地址为0则返回网关下所有北京院电源信息
	{
		k += DeviceManger.ReadAllRecord_PekingPower(&Net.Buf[t]);
		t += k;
	}
	else//返回单个北京院电源信息
	{
		k += DeviceManger.ReadRecord_PekingPower(&Net.Buf[t],addr);
		t += k;
	}
	
	Net.Buf[10] = k;           //???????
    Net.Buf[11] = k >> 8; 
	
	Net.Buf[6] = t-1; //整个数据帧长度字节
	Net.Buf[7] = (t - 1) >> 8;
	crc = CalCrc16(&Net.Buf[0], t, 0);
	Net.Buf[t++] = crc;
	Net.Buf[t++] = crc >> 8;
	SendSocketData(Net.Socket, &Net.Buf[0], t);
}

void AnswerExcuteList(void)
{
	u8 i;
	u16 k,crc,t;
	t = 8;
	k = 3;
	MakeFrameHeader();
	Net.Buf[t++] = 0x01;
	Net.Buf[t++] = 0xEA; //协议编码
	Net.Buf[t++] = 0; //编码长度低八位  编码 + 长度2字节 + 网关信息8字节 + N*5
	Net.Buf[t++] = 0; //编码长度高八位, 最后更新
	for(i=0;i < MaxTriggerNum;i++)
	{
		Net.Buf[t++] = Excute[i].TriggerAddr;
		k++;
		Net.Buf[t++] = Excute[i].ExcuteAddr;
		k++;
		Net.Buf[t++] = Excute[i].Trigger;
		k++;
	}
	
	Net.Buf[10] = k;           //???????
    Net.Buf[11] = k >> 8; 
	
	Net.Buf[6] = t + 2; //整个数据帧长度字节
	Net.Buf[7] = (t + 2) >> 8;
	Net.Buf[t++] = Sys.Crc;
	Net.Buf[t++] = Sys.Crc >> 8;
	crc = CalCrc16(&Net.Buf[0], t, 0);
	
	Net.Buf[t++] = crc;
	Net.Buf[t++] = crc >> 8;
	SendSocketData(Net.Socket, &Net.Buf[0], t);
}

void AnswerSetDebugSocket(u8 cmd)
{
	if(cmd == 0x00)
		Sys.DebugSocket = 0xFF;
	else
		Sys.DebugSocket = Net.Socket;
}

void AnswerGetIPDInfo(u8 addr)
{
	u16 crc, t,k,m=0;
	MakeFrameHeader();
	t = 8;
	Net.Buf[t++] = 0x01;
	Net.Buf[t++] = 0xEF; //协议编码
	k = t;
	Net.Buf[t++] = 0; 
	Net.Buf[t++] = 0;
	m = DeviceManger.ReadDetailRecord_IPD_ZJM(&Net.Buf[t],addr);
	t += m;
	Net.Buf[t++] = Sys.Crc;
	Net.Buf[t++] = Sys.Crc >> 8;
	
	Net.Buf[k] = m+3;
	Net.Buf[k+1] = (m+3)>>8;
	Net.Buf[6] = t + 2; //整个数据帧长度字节
	Net.Buf[7] = (t + 2) >> 8;

	crc = CalCrc16(&Net.Buf[0], t, 0);
	Net.Buf[t++] = crc;
	Net.Buf[t++] = crc >> 8;
	SendSocketData(Net.Socket, &Net.Buf[0], t);
}

void AnswerCycleSec(void)
{
	u16 i,j,crc,t,len;
     
	if(Voice.W >= Voice.R)
		len = Voice.W - Voice.R;
	else
		len = (MaxVoiceLen - Voice.R + Voice.W);
     
	MakeFrameHeader();
	t = 8;
     
	Net.Buf[t++] = 0x01;
	Net.Buf[t++] = 0x69;
	Net.Buf[t++] = 0;
	Net.Buf[t++] = 0;
     
	Net.Buf[t++] = len;            // 缓冲区占有情况
	Net.Buf[t++] = len >> 8;
	Net.Buf[t++] = Net.VoiceCnt;   // 帧序数
	Net.Buf[t++] = Net.VoiceCnt >> 8;
     
	j = 0;
	for(i = 0;i < MaxDeviceNum;i ++)
	{
		if((!(Device[i].Status & 0x80))  && (Device[i].Flag & 0x01))
		{
			Net.Buf[t++] = i + 1;                //????
			Net.Buf[t++] = Device[i].Name;       //??
			Net.Buf[t++] = Device[i].Status;    //??
			Net.Buf[t++] = Device[i].Buf[0];    //?????
			Net.Buf[t++] = Device[i].Buf[1];    //?????
			j++;
		}
	}
	j = j*5 + 7;
	Net.Buf[10] = j;
	Net.Buf[11] = j >> 8;

	Net.Buf[6] = t + 2;
	Net.Buf[7] = (t+2) >> 8;

	crc = CalCrc16(&Net.Buf[0],t,0);
	Net.Buf[t++] = crc;
	Net.Buf[t++] = crc >> 8; 
	SendSocketData(Sys.VoiceSocket,&Net.Buf[0],t);
}

void AnswerErrorBitsInfo(u8 *buf,u16 len)
{
     u16 i,crc,t;
     MakeFrameHeader();
     t = 8;
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x63;        //????  
     Net.Buf[t++] = 0;          //???????  ?? + ??2?? + N*5
     Net.Buf[t++] = 0;           //???????, ????
     
     for(i = 0;i < len;i ++)
          Net.Buf[t++] = buf[i];
     
     Net.Buf[10] = len+3;    // ???????
     Net.Buf[11] = (len+3) >> 8;

     Net.Buf[6] = t + 2;    //?????????
     Net.Buf[7] = (t+2) >> 8;
     
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerUnRegisterInfo(void)
{
     u16 i,j,crc,t;
     MakeFrameHeader();
     t = 8;
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x62;        //????  
     Net.Buf[t++] = 0;          //???????  ?? + ??2?? + N*5
     Net.Buf[t++] = 0;           //???????, ????
     j = 0;                      //????????,?????????????
     for(i = 0;i < MaxDeviceNum;i ++)
     {
          if(Device[i].Flag & 0x02)    //?????
          {
               Net.Buf[t++] = i + 1;                //????
               Net.Buf[t++] = Device[i].Name;       //??
               j ++;
          }
     }
     j = j*2 + 3;
     Net.Buf[10] = j;    // ???????
     Net.Buf[11] = j >> 8;

     Net.Buf[6] = t + 2;    //?????????
     Net.Buf[7] = (t+2) >> 8;
     
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerCrcInfo(void)
{
     u16 i,j,crc,t;
     MakeFrameHeader();
     t = 8;
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x68;        //????  
     Net.Buf[t++] = 0;          //???????  ?? + ??2?? + N*5
     Net.Buf[t++] = 0;           //???????, ????
     j = 0;                      //????????,?????????????
     for(i = 0;i < MaxDeviceNum;i ++)
     {
          if(Device[i].Flag & 0x01)    //????
          {
               Net.Buf[t++] = i + 1;                //????
               Net.Buf[t++] = Device[i].Name;       //crc
               Net.Buf[t++] = Device[i].Crc;       //crc
               Net.Buf[t++] = Device[i].RecTimes;  //????
               Net.Buf[t++] = Device[i].Warn;      // ??????
               j ++;
          }
     }
     j = j*5 + 3;
     Net.Buf[10] = j;    // ???????
     Net.Buf[11] = j >> 8;

     Net.Buf[6] = t + 2;    //?????????
     Net.Buf[7] = (t+2) >> 8;
     
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerCrcFail(void)
{
     u16 i,j,crc,t;
     MakeFrameHeader();
     t = 8;
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x66;        //????  
     Net.Buf[t++] = 0;          //???????  ?? + ??2?? + N*5
     Net.Buf[t++] = 0;           //???????, ????
     j = 0;                      //????????,?????????????
     for(i = 0;i < MaxDeviceNum;i ++)
     {
          if(Device[i].CrcCnt > 5)    //????
          {
               Net.Buf[t++] = i + 1;                //????
               Net.Buf[t++] = Device[i].Name;       //??
               j ++;
          }
     }
     j = j*2 + 3;
     Net.Buf[10] = j;    // ???????
     Net.Buf[11] = j >> 8;

     Net.Buf[6] = t + 2;    //?????????
     Net.Buf[7] = (t+2) >> 8;
     
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerResetCrcFail(void)
{
     Device[Net.Buf[12]-1].CrcCnt = 0;
     AnswerAck(0);
}

void AnswerBasePara(void)
{
     u8 t;
     u16 crc;
     t = 8;
     ReadFactoryTime();
     MakeFrameHeader();
     
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x71;
     Net.Buf[t++] = 0x0E;
     Net.Buf[t++] = 0x00;
     
     Net.Buf[t++] = Sys.Vol;
     Net.Buf[t++] = Sys.Addr;
     Net.Buf[t++] = Sys.ResetTimes;
     
     TimeChange(&Net.Buf[t]);
     t += 4;
     
     Net.Buf[t++] = ConfigBuf[0];
     Net.Buf[t++] = ConfigBuf[1];
     Net.Buf[t++] = ConfigBuf[2];
     Net.Buf[t++] = ConfigBuf[3];
     
     Net.Buf[6] = t+2;
     Net.Buf[7] = 0; 
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerSoftVerb(void)
{
     u8 t;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = 0x72;           //????
     Net.Buf[t++] = 0x04;           //???????
     Net.Buf[t++] = 0x00;           //???????
     
     Net.Buf[t++] = SoftVerb;
     // ???????
     Net.Buf[6] = t+2;        // ???
     Net.Buf[7] = 0;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}
// ??IP??????
void AnswerIpAddr(void)
{
     u8 t,i;
     u16 crc;
     t = 8;
     ReadDefaultIpAddr();
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;
     Net.Buf[t++] = 0x70;
     Net.Buf[t++] = 0x10;           //???????
     Net.Buf[t++] = 0x00;           //???????
     
	 FramReadBurstByte(NetIpAddr,ConfigBuf,6);
     for(i = 0;i < 4;i ++)               //IP??????
          Net.Buf[t++] = ConfigBuf[i];
     Net.Buf[t++] = ConfigBuf[i++];
     Net.Buf[t++] = ConfigBuf[i++];
     ReadDefaultGateWayIp();            //????
     for(i = 0;i < 4;i ++)               //IP??????
          Net.Buf[t++] = Net.GatewayIP[i];
     for(i = 0;i < 4;i ++)               // UDP ??????
          Net.Buf[t++] = Net.UdpDipr[i];
     Net.Buf[t++] = Net.UdpPort;
     Net.Buf[t++] = Net.UdpPort >> 8;
     
     // ???????
     Net.Buf[6] = t+2;        // ???
     Net.Buf[7] = 0;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerForBootLoader(void)
{
     u8 i;
     for(i = 0;i < 6;i ++)
          ConfigBuf[i] = Net.Buf[12+i];
     WriteConfig(BootLoaderAddr,6);
     AnswerAck(0);
     WriteSocket1Byte(0,Sn_CR,CLOSENET);
     WriteSocket1Byte(1,Sn_CR,CLOSENET);
     while(1);                             // ??
}

void AnswerPowerInfo(u8 addr)
{
     u8 i,j;
     u16 crc,k,t;
     t = 8;
     k = 3;
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = 0x94;           //????
     Net.Buf[t++] = 0x00;           //???????
     Net.Buf[t++] = 0x00;           //???????
     
     if(addr == 0)       // ???????????
     {
          for(i = 0;i < MaxPowerNum;i ++)
          {
               if(Power[i].Addr == 0)
                    break;
               Net.Buf[t++] = Power[i].Addr;
               for(j = 0;j < 46;j ++)
                    Net.Buf[t++] = Power[i].BatInfo[j];
               k += 47;
          }
     }
     else
     {
          i = CheckPowerIndex(addr);
          if(i != 0xFF)                 // ???????
          {
               Net.Buf[t++] = Power[i].Addr;
               for(j = 0;j < 46;j ++)
                    Net.Buf[t++] = Power[i].BatInfo[j];
               k += 47;
          }
     }
     
     Net.Buf[10] = k;           //???????
     Net.Buf[11] = k >> 8; 
     
     // ???????
     Net.Buf[6] = t+2;                 // ???
     Net.Buf[7] = (t+2) >> 8;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerAck(u8 flag)
{
     u8 t;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;           //????
     if(flag == 0)
     {
          Net.Buf[t++] = ACK;            //????
          Net.Buf[t++] = 0x03;           //???????
          Net.Buf[t++] = 0x00;           //???????
     }
     else                                //???...S
     {
          Net.Buf[t++] = PREPARE;
          Net.Buf[t++] = 0x04;           //???????
          Net.Buf[t++] = 0x00;           //???????
          Net.Buf[t++] = Sys.Delay;
     }
     Net.Buf[6] = t+2;              //???
     Net.Buf[7] = 0;                //???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8;
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}
void AnswerWrongAck(void)
{
     u8 t;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = WRONGACK;       //????
     Net.Buf[t++] = 0x03;           //???????
     Net.Buf[t++] = 0x00;           //???????
     Net.Buf[6] = t+2;              //???
     Net.Buf[7] = 0;                //???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerCheckCrcCntInfo(void)
{
     u8 t;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = 0x64;            //????
     Net.Buf[t++] = 0x04;           //???????
     Net.Buf[t++] = 0x00;           //???????
     Net.Buf[t++] = Sys.CrcCnt;
     Net.Buf[6] = t+2;              //???
     Net.Buf[7] = 0;                //???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}
void AnswerCleanCrcCntInfo(void)
{
     Sys.CrcCnt = 0;
     WriteConfig(NetCrcCntAddr,1);
     AnswerAck(0);
}

void AnswerSetTime(void)
{
     Time.Buf[0] = Net.Buf[12];           // ?
     Time.Buf[1] = Net.Buf[13] & 0x1f;      //??????,;
     Time.Buf[2] = Net.Buf[14];          //?
     Time.Buf[3] = Net.Buf[13] >> 5;     //date
     Time.Buf[4] = Net.Buf[15] & 0x3f;   //?? 6
     Time.Buf[5] = Net.Buf[16];
     Time.Buf[6] = Net.Buf[17];
     WriteTime();
     AnswerAck(0);
}

void AnswerFactoryTime(void)
{
     u8 t;
     u32 time;
     Net.Buf[15] &= 0x1F;
     for(t = 12;t < 18;t ++)
          Net.Buf[t] = (Net.Buf[t]/16)*10+Net.Buf[t]%16;
     t = 0;
     time = 0;
     time = Net.Buf[12];    //?
     time <<= 4;
     time += Net.Buf[13];   //?
     time <<= 5;
     time += Net.Buf[14];   //?
     time <<= 5;
     time += Net.Buf[15];   //??
     time <<= 6;
     time += Net.Buf[16];   //?
     time <<= 6;
     time += Net.Buf[17];   //?
     ConfigBuf[0] = time;
     ConfigBuf[1] = time>>8;
     ConfigBuf[2] = time>>16;
     ConfigBuf[3] = time>>24;
     WriteConfig(FactoryTimeAddr,4);
     AnswerAck(0);
}

void AnswerSetMac(void)
{
     u8 i;
     for(i = 0;i < 6;i ++)              //??IP??????
          ConfigBuf[i] = Net.Buf[12+i];
     WriteConfig(MacBufAddr,6);
     AnswerAck(0);
     while(1);
}

void AnswerCheckMac(void)
{
     u8 t,i;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = 0x88;           //????
     Net.Buf[t++] = 0x09;           //???????
     Net.Buf[t++] = 0x00;           //???????
     
     for(i = 0;i < 6;i ++)
          Net.Buf[t++] = Net.MacAddr[i];
     
     // ???????
     Net.Buf[6] = t+2;        // ???
     Net.Buf[7] = 0;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerSetNetFilterTime(void)
{
     Sys.FilterTime = Net.Buf[12];
     Sys.FilterTime <<= 8;
     Sys.FilterTime += Net.Buf[13];
     UpdateFilterTime();
     AnswerAck(0);
}

void AnswerCheckNetFilterTime(void)
{
     u8 t;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = 0x8A;           //????
     Net.Buf[t++] = 0x05;           //???????
     Net.Buf[t++] = 0x00;           //???????
     
     Net.Buf[t++] = Sys.FilterTime;
     Net.Buf[t++] = Sys.FilterTime >> 8;
     
     // ???????
     Net.Buf[6] = t+2;        // ???
     Net.Buf[7] = 0;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void AnswerSetBiaoJiao(void)
{
    ;
}

void AnswerReseetNet(void)
{
     AnswerAck(0);
     while(1);
}

void AnswerSetIp(void)
{
     u8 i; 
     for(i = 0;i < 6;i ++)
          ConfigBuf[i] = Net.Buf[12+i];
     WriteConfig(NetIpAddr,6);
     for(i = 0;i < 4;i ++)
          ConfigBuf[i] = Net.Buf[18+i];
     WriteConfig(GateWayIpAddr,4);
     ConfigBuf[0] = Net.Buf[22];
     WriteConfig(NetAddrAddr,1);
     AnswerAck(0);
     while(1);              //????
}

void AnawerSetUdp(void)
{
     u8 i; 
     for(i = 0;i < 6;i ++)              //??IP??????
          ConfigBuf[i] = Net.Buf[12+i];
     WriteConfig(NetUdpIpAddr,6);
     Net.UdpDipr[0] = Net.Buf[12];
     Net.UdpDipr[1] = Net.Buf[13];
     Net.UdpDipr[2] = Net.Buf[14];
     Net.UdpDipr[3] = Net.Buf[15];
     Net.UdpPort = Net.Buf[17];
     Net.UdpPort <<= 8;
     Net.UdpPort += Net.Buf[16];
     AnswerAck(0);
}

void AnswerSetResetTimes(void)
{
     Sys.ResetTimes = 0;
     ConfigBuf[0] = 0;
     WriteConfig(ResetTimesAddr,1);
     AnswerAck(0);
}

void AnswerCleanRWptr(void)
{
     Sys.R = FramBegin;
     Sys.W = FramBegin;
     Sys.Shift = 0;        //?????,??????????,????
     UpdateWptr();
     UpdateRptr();
     AnswerAck(0);
}
// CAN????????
void CanInfoToNet(u8 cmd,u8 *buf,u8 len)
{
     u8 t,i;
     u16 crc;
     t = 8;
     MakeFrameHeader();
     Net.Buf[t++] = 0x01;           //????
     Net.Buf[t++] = cmd;           //????
     Net.Buf[t++] = len+3;           //???????
     Net.Buf[t++] = 0x00;           //???????
     for(i = 0;i < len;i ++)
          Net.Buf[t++] = buf[i];
     // ???????
     Net.Buf[6] = t+2;        // ???
     Net.Buf[7] = 0;          // ???
     crc = CalCrc16(&Net.Buf[0],t,0);
     Net.Buf[t++] = crc;
     Net.Buf[t++] = crc >> 8; 
     SendSocketData(Net.Socket,&Net.Buf[0],t);
}

void SendDebugData(u32 ID,u8* buf,u8 len,u8 type)
{
	u8 UpLoadCanBuf[255] = {0};
	if(Sys.DebugSocket!=0xFF)
	{
		switch(type)
		{
		case 0://发送CAN原始数据
			UpLoadCanBuf[8] = len;
			UpLoadCanBuf[9] = 0xEB;
			UpLoadCanBuf[10] = ID >> 24;
			UpLoadCanBuf[11] = ID >> 16;
			UpLoadCanBuf[12] = ID >> 8;
			UpLoadCanBuf[13] = ID ;
			BufCopy(&UpLoadCanBuf[14], buf, len);
			SendSocketData(Sys.DebugSocket, UpLoadCanBuf, 4 + len + 10);
			break;
		case 1:
			UpLoadCanBuf[6] = Device[Init.DestAddr - 1].Name;
			UpLoadCanBuf[7] = Init.DestAddr;
			UpLoadCanBuf[8] = len;
			UpLoadCanBuf[9] = 0xCC;
			BufCopy(&UpLoadCanBuf[10],buf,len);
			SendSocketData(Sys.DebugSocket, UpLoadCanBuf, len + 10);
			break;
		case 2:
			UpLoadCanBuf[8] = len;
			UpLoadCanBuf[9] = 0xCD;
			UpLoadCanBuf[10] = ID >> 24;
			UpLoadCanBuf[11] = ID >> 16;
			UpLoadCanBuf[12] = ID >> 8;
			UpLoadCanBuf[13] = ID ;
			BufCopy(&UpLoadCanBuf[14], buf, len);
			SendSocketData(Sys.DebugSocket, UpLoadCanBuf, 4 + len + 10);
			break;
		case 3:
			UpLoadCanBuf[8] = len;
			UpLoadCanBuf[9] = 0xCE;
			UpLoadCanBuf[10] = ID >> 24;
			UpLoadCanBuf[11] = ID >> 16;
			UpLoadCanBuf[12] = ID >> 8;
			UpLoadCanBuf[13] = ID ;
			BufCopy(&UpLoadCanBuf[14], buf, len);
			SendSocketData(Sys.DebugSocket, UpLoadCanBuf, 4 + len + 10);
			break;
		}
		
	}
}
