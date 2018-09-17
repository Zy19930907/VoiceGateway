#include "FmInit.h"
#include "Config.h"
#include "W5500.h"
#include "SD2068.h"
#include "CanPro.h"

u8 ConfigBuf[10];
CSensor Device[MaxDeviceNum];
CExcute Excute[MaxTriggerNum];
extern CPower  Power[MaxPowerNum];
extern vu16 SYS_TICK,SYS_TickSec;
extern CNet Net;
extern CSys Sys;
extern CTime Time;
extern CSendInit Init;

void WriteConfig(u8 addr,u8 len)
{
     u16 crc;
     crc = CalCrc16(ConfigBuf,len,0);
     ConfigBuf[len++] = crc;
     ConfigBuf[len++] = crc >> 8;
     FramWriteBurstByte(addr,ConfigBuf,len);
}

u8  ConfigCheck(u8 addr,u8 len)
{
     FramReadBurstByte(addr,ConfigBuf,len);
     if(CRC16Check(ConfigBuf,len))
          return 1;
     else
          return 0;
}

void ReadFactoryTime(void)
{
     if(!ConfigCheck(FactoryTimeAddr,6))
     {
          ConfigBuf[0] = 0;
          ConfigBuf[0] = 0;
          ConfigBuf[0] = 0;
          ConfigBuf[0] = 0;
     }
}

void ReadDefaultAddr(void)
{
     /*if(ConfigCheck(NetAddrAddr,3))
          Sys.Addr = ConfigBuf[0];
     else
          Sys.Addr = 61;
	*/
	Sys.Addr = 12;
}

//#define IP_107
//#define IP_110
//#define IP_112
//#define IP_117
//#define IP_120
//#define IP_123
//#define IP_125
//#define IP_127
//#define IP_130
//#define IP_133
//#define IP_136
//#define IP_138
//#define IP_140
//#define IP_142
//#define IP_144
//#define IP_146
//#define IP_148
//#define IP_150
//#define IP_154
//#define IP_155
//#define IP_157
//#define IP_159
//#define IP_161
//#define IP_163
//#define IP_168
//#define IP_171
//#define IP_172
//#define IP_173
//#define IP_174
//#define IP_175
//#define IP_176
//#define IP_177
#define IP_199
//#define IP_211
//#define IP_225
//#define IP_102

void ReadDefaultIpAddr(void)
{
	u8 i;
	#ifdef IP_107
		u8 mac[6] = {0x00,0x22,0x70,0x02,0xBF,0x0C}; //--110
		Net.IPAddr[3] = 107;
	#endif
		
	#ifdef IP_110
		u8 mac[6] = {0x00,0x22,0x70,0x01,0xB4,0xB0}; //--110
		Net.IPAddr[3] = 110;
	#endif
	#ifdef IP_112
		u8 mac[6] = {0x00,0x00,0xEB,0x02,0x0F,0xB1};   //--112
		Net.IPAddr[3] = 112;
	#endif
	#ifdef IP_117
		u8 mac[6] = {0x00,0x6B,0xCF,0x03,0xB1,0xB2}; //--117
		Net.IPAddr[3] = 117;
	#endif
	#ifdef IP_120
		u8 mac[6] = {0x00,0x00,0xEC,0x04,0x0D,0xB3}; //--120
		Net.IPAddr[3] = 120;
	#endif
	#ifdef IP_123
		u8 mac[6] = {0x00,0x23,0x72,0x05,0xB4,0xB4}; //--123
		Net.IPAddr[3] = 123;
	#endif
	#ifdef IP_125
		u8 mac[6] = {0x00,0x22,0x6F,0x06,0xB8,0xB5}; //--125
		Net.IPAddr[3] = 125;
	#endif
	#ifdef IP_127
		u8 mac[6] = {0x00,0x00,0xEB,0x07,0x00,0xB6}; //--127
		Net.IPAddr[3] = 127;
	#endif
	#ifdef IP_130
		u8 mac[6] = {0x00,0x00,0xED,0x08,0x0C,0xB7}; //--130
		Net.IPAddr[3] = 130;
	#endif
	#ifdef IP_133
		u8 mac[6] = {0x00,0x1F,0x72,0x09,0xB4,0xB8}; //--133
		Net.IPAddr[3] = 133;
	#endif
	#ifdef IP_136
		u8 mac[6] = {0x00,0x1F,0x71,0x0A,0xB4,0xB9}; //--136
		Net.IPAddr[3] = 136;
	#endif
	#ifdef IP_138
		u8 mac[6] = {0x00,0x21,0x6F,0x0B,0x91,0xBA}; //--138
		Net.IPAddr[3] = 138;
	#endif
	#ifdef IP_140
		u8 mac[6] = {0x00,0x20,0xDD,0x0C,0xAA,0xBB}; //--140
		Net.IPAddr[3] = 140;
	#endif
	#ifdef IP_142
		u8 mac[6] = {0x00,0x0E,0x6F,0x0D,0xB4,0xBC}; //--142
		Net.IPAddr[3] = 142;
	#endif
	#ifdef IP_144
		u8 mac[6] = {0x00,0x0E,0x7B,0x0E,0xB4,0xBD}; //--144
		Net.IPAddr[3] = 144;
	#endif
	#ifdef IP_146
		u8 mac[6] = {0x00,0x0E,0x7B,0x0F,0x96,0xBE}; //--146
		Net.IPAddr[3] = 146;
	#endif
	#ifdef IP_148
		u8 mac[6] = {0x00,0x0C,0x7C,0x10,0x96,0xBF}; //--148
		Net.IPAddr[3] = 148;
	#endif
	#ifdef IP_150
		u8 mac[6] = {0x00,0x0B,0x7C,0x11,0x96,0xC0}; //--150
		Net.IPAddr[3] = 150;
	#endif
	#ifdef IP_154
		u8 mac[6] = {0x00,0x1E,0x71,0x12,0xB4,0xC1}; //--154
		Net.IPAddr[3] = 154;
	#endif
	#ifdef IP_155
		u8 mac[6] = {0x00,0x0F,0x7C,0x13,0x96,0xC2}; //--155
		Net.IPAddr[3] = 155;
	#endif
	#ifdef IP_157
		u8 mac[6] = {0x00,0x10,0x7C,0x14,0x96,0xC3}; //--157
		Net.IPAddr[3] = 157;
	#endif
	#ifdef IP_159
		u8 mac[6] = {0x00,0x0D,0x7C,0x15,0x96,0xC4}; //--159
		Net.IPAddr[3] = 159;
	#endif
	#ifdef IP_161
		u8 mac[6] = {0x00,0x0E,0x7C,0x16,0x96,0xC5}; //--161
		Net.IPAddr[3] = 161;
	#endif
	#ifdef IP_163
		u8 mac[6] = {0x00,0x11,0x7C,0x17,0x96,0xC6}; //--163
		Net.IPAddr[3] = 163;
	#endif
	#ifdef IP_168
		u8 mac[6] = {0x00,0x11,0x7C,0x18,0x96,0xC7}; //--168
		Net.IPAddr[3] = 168;
	#endif
	#ifdef IP_171
		u8 mac[6] = {0x00,0x23,0x72,0x19,0xB4,0xC8}; //--171
		Net.IPAddr[3] = 171;
	#endif
	
	#ifdef IP_172
		u8 mac[6] = {0x00,0x23,0x72,0x1A,0x32,0xC9}; //--172
		Net.IPAddr[3] = 172;
	#endif
		
		
	#ifdef IP_173
		u8 mac[6] = {0x00,0x23,0x72,0x1B,0x32,0xCA}; //--173
		Net.IPAddr[3] = 173;
	#endif
		
	#ifdef IP_174
		u8 mac[6] = {0x00,0x23,0x72,0x1C,0x32,0xCB}; //--174
		Net.IPAddr[3] = 174;
	#endif
		
	#ifdef IP_175
		u8 mac[6] = {0x00,0x23,0x72,0x1D,0x32,0xCC}; //--175
		Net.IPAddr[3] = 175;
	#endif
		
	#ifdef IP_176
		u8 mac[6] = {0x00,0x23,0x72,0x1E,0x32,0xCD}; //--176
		Net.IPAddr[3] = 176;
	#endif
		
	#ifdef IP_177
		u8 mac[6] = {0x00,0x23,0x72,0x1F,0x32,0xCE}; //--177
		Net.IPAddr[3] = 177;
	#endif
		
	#ifdef IP_199
		u8 mac[6] = {0x00,0x22,0x6F,0x20,0x91,0xCF}; //--199
		Net.IPAddr[3] = 199;
	#endif

	#ifdef IP_102
		u8 mac[6] = {0x00,0x23,0x77,0x21,0xA4,0xD0}; //--168
		Net.IPAddr[3] = 102;
	#endif
	#ifdef IP_211
		u8 mac[6] = {0x00,0x23,0x77,0x22,0xA4,0xD1}; //--211
		Net.IPAddr[3] = 211;
	#endif
		
	#ifdef IP_225
		u8 mac[6] = {0x00,0x11,0x7C,0x23,0x96,0xD2}; //--168
		Net.IPAddr[0] = 192;
		Net.IPAddr[1] = 168;
		Net.IPAddr[2] = 1;
		Net.IPAddr[3] = 225;
		Net.S0_SourcePort = 5000;

		Net.GatewayIP[0] = 192;
		Net.GatewayIP[1] = 168;
		Net.GatewayIP[2] = 1;
		Net.GatewayIP[3] = 1;
	#endif
	
     /*if(ConfigCheck(NetIpAddr,8))
     {
          Net.IPAddr[0] = ConfigBuf[0];
          Net.IPAddr[1] = ConfigBuf[1];
          Net.IPAddr[2] = ConfigBuf[2];
          Net.IPAddr[3] = ConfigBuf[3];
          Net.S0_SourcePort = ConfigBuf[5];
          Net.S0_SourcePort <<= 8;
          Net.S0_SourcePort += ConfigBuf[4];
     }
     else
     {
          Net.IPAddr[0] = 192;
          Net.IPAddr[1] = 168;
          Net.IPAddr[2] = 205;
          Net.IPAddr[3] = 200;
          Net.S0_SourcePort = 5000;
     }*/
	#ifndef IP_225
	Net.IPAddr[0] = 192;
	Net.IPAddr[1] = 168;
	Net.IPAddr[2] = 205;

	Net.S0_SourcePort = 5000;

	Net.GatewayIP[0] = 192;
	Net.GatewayIP[1] = 168;
	Net.GatewayIP[2] = 205;
	Net.GatewayIP[3] = 254;
	#endif

			//配置写死
	Sys.Addr = Net.IPAddr[3] % 100;
	BufCopy(Net.MacAddr,&mac[0],6);
	for(i = 0;i < 4;i ++)
          ConfigBuf[i] = Net.IPAddr[i];
	ConfigBuf[4] = (u8)5000;
	ConfigBuf[5] = (u8)(5000 >> 8);
    WriteConfig(NetIpAddr,6);//将IP地址写入EEPROM
	
}
void ReadDefaultGateWayIp(void)
{
    if(ConfigCheck(GateWayIpAddr,6))
     {
          Net.GatewayIP[0] = ConfigBuf[0];
          Net.GatewayIP[1] = ConfigBuf[1];
          Net.GatewayIP[2] = ConfigBuf[2];
          Net.GatewayIP[3] = ConfigBuf[3];
     }
     else
     {
          Net.GatewayIP[0] = 192;
          Net.GatewayIP[1] = 168;
          Net.GatewayIP[2] = 205;
          Net.GatewayIP[3] = 254;
    }
}

//  ?UDP??????
void ReadDefaultUdp(void)
{
     if(ConfigCheck(NetUdpIpAddr,8))
     {
          Net.UdpDipr[0] = ConfigBuf[0];
          Net.UdpDipr[1] = ConfigBuf[1];
          Net.UdpDipr[2] = ConfigBuf[2];
          Net.UdpDipr[3] = ConfigBuf[3];
          Net.UdpPort = ConfigBuf[5];
          Net.UdpPort <<= 8;
          Net.UdpPort += ConfigBuf[4];
     }
     else
     {
          Net.UdpDipr[0] = 0;
          Net.UdpDipr[1] = 0;
          Net.UdpDipr[2] = 0;
          Net.UdpDipr[3] = 0;
          Net.UdpPort = 0;
     }
}

void ReadDefaultMAC(void)
{
     u8 i;
     u16 t;
     if(ConfigCheck(MacBufAddr,8))
     {
          for(i = 0;i < 6;i ++)
               Net.MacAddr[i] = ConfigBuf[i];
     }
     else
     {
          ConfigBuf[0] = Sys.Addr << 1;
          ConfigBuf[1] = Sys.ResetTimes;
          ConfigBuf[2] = SYS_TICK;
          ConfigBuf[3] = SYS_TICK >> 8;
          t = CalCrc16(ConfigBuf,4,0);
          ConfigBuf[4] = t;
          ConfigBuf[5] = t >> 8;
          for(i = 0;i < 6;i ++)
               Net.MacAddr[i] = ConfigBuf[i];
          WriteConfig(MacBufAddr,6);
     }
}

void ReadDefaultResetTimes(void)
{
     if(ConfigCheck(ResetTimesAddr,3))
          Sys.ResetTimes += ConfigBuf[0];
     else
          Sys.ResetTimes = 0;
}

void ReadDefaultReadPtr(void)
{
     if(ConfigCheck(ReadPointerAddr,4))
     {
          Sys.R = ConfigBuf[1];
          Sys.R <<= 8;
          Sys.R += ConfigBuf[0];
     }
     else
          Sys.R = FramBegin;
}

void ReadDefaultWritePtr(void)
{
     if(ConfigCheck(WritePointerAddr,4))
     {
          Sys.W = ConfigBuf[1];
          Sys.W <<= 8;
          Sys.W += ConfigBuf[0];
     }
     else
          Sys.W = FramBegin;
}

void ReadDefaultFilterTime(void)
{
     if(ConfigCheck(FilterTimeAddr,4))
     {
          Sys.FilterTime = ConfigBuf[1];
          Sys.FilterTime <<= 8;
          Sys.FilterTime += ConfigBuf[0];
     }
     else
          Sys.FilterTime = 20;
}

void UpdateFilterTime(void)
{
     ConfigBuf[0] = Sys.FilterTime;
     ConfigBuf[1] = Sys.FilterTime >> 8;
     WriteConfig(FilterTimeAddr,2);
}

void ReadDefaultCrcCnt(void)
{
     if(ConfigCheck(NetCrcCntAddr,3))
          Sys.CrcCnt = ConfigBuf[0];
     else
          Sys.CrcCnt = 0;
}

void UpdateRptr(void)
{
     ConfigBuf[0] = Sys.R;
     ConfigBuf[1] = Sys.R >> 8;
     WriteConfig(ReadPointerAddr,2);
}

void UpdateWptr(void)
{
     ConfigBuf[0] = Sys.W;
     ConfigBuf[1] = Sys.W >> 8;
     WriteConfig(WritePointerAddr,2);
}

void ReadDefaultInitLen(void)
{
     if(ConfigCheck(InitLenAddr,4))
     {
          Sys.InitLen = ConfigBuf[1];
          Sys.InitLen <<= 8;
          Sys.InitLen += ConfigBuf[0];
     }
     else
          Sys.InitLen = 0;
}

void ReadDefaultInitCrc(void)
{
     if(ConfigCheck(InitCrcAddr,4))
     {
          Sys.Crc = ConfigBuf[1];
          Sys.Crc <<= 8;
          Sys.Crc += ConfigBuf[0];
     }
     else
          Sys.Crc = 0;
}

void UpdateDeviceInfo(void)
{
     u8 i;//,buf[11];
     ReadTime();     
     
     if(Sys.InitLeft)
          return;
	 
     for(i = 0;i < MaxDeviceNum;i ++)
     {
		if(!(Device[i].Flag & 0x03))
			continue;
		if(Device[i].Status & 0x80)
			continue;
		if(Device[i].CheckCnt > 5)         // 广播连续5次都没有响应，则证明已经中断
			Device[i].Status |= 0x10;
		if((SecTickDiff(Device[i].Tick) < Sys.FilterTime) && (!(Device[i].Flag & 0x04)))
			continue;
//		if((SecTickDiff(Device[i].Tick) < Sys.ActFilterTime) && (Device[i].Flag & 0x04))
//			continue;
//		if((SecTickDiff(Device[i].Tick) > Sys.ActFilterTime) && (Device[i].Flag & 0x04))
//			CheckLianDongInfo(i+1,0x10);
//		if(Device[i].Name == 0x28)
//			continue;
		
//		if(Sys.BroadCastLeft)        
//		{
//            Sys.BroadCastLeft--;
//			if(Device[i].Name == 0x28 || Device[i].Name == 0x24)// 有广播播放的时候，不计算广播读卡器断线
//				return;
//		}
//		
		if((Device[i].Name == 0x28) || (Device[i].Name == 0x24)) //广播和读卡器按照原有离线管理机制
		{
			Device[i].Status |= 0x80;
			Device[i].Flag &= ~0x02;
			Device[i].Buf[0] = 0;
			Device[i].Buf[1] = 0;
		}
 /*         
          buf[0] = i + 1;
          buf[1] = Device[i].Name;
          buf[2] = 0x80; 
          buf[3] = 0;
          buf[4] = 0;
          buf[5] = 0;
          TimeChange(&buf[6]);
          buf[10] = SumAll(buf,10);
          FramWriteBurstByte(Sys.W,buf,11);
          Sys.W += 11;
          if(Sys.W == FramEnd)
               Sys.W = FramBegin;
          UpdateWptr();*/
//		if(buf[1] == PowerName)
//		{
//			UpdatePowerInfo(buf[0]);
//		}
	}
}

void UpdatePowerInfo(u8 addr)
{
     u8 i;
     i = CheckPowerIndex(addr);
     if(i == 0xFF)
          return;
     Power[i].Addr = 0;
}
void ReadDefaultConfig(void)
{
     ReadDefaultAddr();
     ReadDefaultIpAddr();
     ReadDefaultGateWayIp();
     ReadDefaultUdp();
     ReadDefaultMAC();
     ReadDefaultResetTimes();
     ReadDefaultReadPtr();
     ReadDefaultWritePtr();
     ReadDefaultCrcCnt();
     ReadDefaultFilterTime();
     Sys.ResetTimes ++;
     ConfigBuf[0] = Sys.ResetTimes;
     WriteConfig(ResetTimesAddr,1);
     WriteSensorRecord(0,NETRESET);
     ReadDefaultInit();
}


void ReadDeviceDefine(void)
{
     u8 i,j,n,buf[32];
     FramReadBurstByte(BaseInfoAddr,buf,32);
     for(i = 0;i < MaxDeviceNum;i ++)
          Device[i].Flag = 0;
     n = 0;
     for(i = 0;i < 16;i ++)
     {
          for(j = 0;j < 8;j ++)
          {
               if(buf[i] & 0x01)
               {
                    Device[n].Flag = 0x01;
                    Device[n].Name = 0xFF;
                    Device[n].Crc  = 0;
                    Device[n].CrcCnt = 0;
               }
               buf[i] >>= 1;    
               n ++;
          }
     }
}
/*
 * 真正初始化信息域的长度是Sys.InitLen-3(除去命令 自身长度占用的3个字节)
 *  前面32个字节是设备定义域
 * 然后第33个字节是01，代表传感器配置信息域，34,35个字节是长度字节。从第36个字节开始
 * 最后是配置信息（一般指断电器）
*/
// 先计算 配置信息域，如果有断线报警，则修改传感器的报警信息，最后再来计算传感器的crc
// void CalDeviceCrc(void)
// {
// 	u8 crc,k,addr,sensoraddr,trigger,buf[255],n;
// 	u16 i,j,allcnt01,allcnt02,t;
// 	for(k = 0;k < MaxDeviceNum;k ++)    // 先全部清零和清除设备的断线报警状态
// 	{
// 		Device[k].Crc = 0;
// 		Device[k].CrcCnt = 0;
// 		Device[k].Flag &= ~0x04;
//           Device[k].Warn = 0;
// 	}
// 	n = 0;
// 	for(k = 0;k < MaxTriggerNum;k ++)
// 	{
// 		Excute[k].ExcuteAddr = 0;
// 		Excute[k].Trigger = 0;
// 		Excute[k].Warn = 0;
// 		Excute[k].TriggerAddr = 0;
// 	}
// 	if(Sys.InitLen == 0x20)    // 只有设备定义域，没有报警信息域
// 		return;
// 	allcnt01 = Net.Buf[34];
// 	allcnt01 <<= 8;
// 	allcnt01 += Net.Buf[33];
// 	allcnt01 -= 3;    			//除去命令标记和长度自身字节
// 	if(allcnt01 > 1500)
// 		return;
// 	if((allcnt01 % 13) != 0)       
// 		return;
//     // 先计算配置信息中的设备 主要是指断电器,关联读卡器，广播
// 	if(Sys.InitLen-3 == 32+allcnt01)   //没有配置控制信息了，返回
// 		goto CalSensorWarnCrc;
// 	allcnt02 = Net.Buf[32+allcnt01+5];
// 	allcnt02 <<= 8;
// 	allcnt02 += Net.Buf[32+allcnt01+4];
// 	allcnt02 -= 3;
// 	if((allcnt02 % 4) != 0)     //控制信息域不对，则不计算控制信息域
// 		goto CalSensorWarnCrc;
// 	t = 0;
// 	// 断电器的校验按照  关联传感器编号 触发状态  上报......下复
// 	for(i = 0;i < allcnt02;)
// 	{
// 		addr = Net.Buf[38+allcnt01+i];         // 断电器地址
//         trigger = Net.Buf[38+allcnt01+i+1];    // 触发状态
//         sensoraddr = Net.Buf[38+allcnt01+i+2]; // 关联传感器/开关量编号
// 		Excute[n].TriggerAddr = sensoraddr;
// 		Excute[n].ExcuteAddr = addr;
// 		Excute[n].Trigger = trigger;
// 		n ++;
// 		if(Net.Buf[38+allcnt01+i+3] == 0x01)   // 开关量
// 		{
// 			trigger |= 0x80;
// 			Device[sensoraddr-1].Crc |= 0x01;
// 			if(trigger & 0x10)
// 			{
// 				Device[sensoraddr-1].Crc = (trigger>>3)+1;      // D3 1态 D2 0态 D1 断线控 D0 参与控制（其实D0无太大意义，只是为了兼容以前老版本）
//                trigger |= 0x80;
//                if(trigger & 0x10)               // 断线控制
//                     Device[sensoraddr-1].Flag |= 0x04;
// 			}
// 		}
// 		  buf[t++] = sensoraddr;     
//           buf[t++] = trigger;
//           for(j = 0;j < allcnt01;)               // 如果是开关量，则找不到报警数据
//           {
//                if(sensoraddr == Net.Buf[35+j])   //地址相同,填入12个字节的初始化参数
//                {
//                     if(trigger & 0x10)          //参与断线控制 上报的最高位D15置1
//                          Device[sensoraddr-1].Flag |= 0x04;   // 将传感器 Flag 的D2置1，表示该传感器参与断线控制
//                     for(k = 0;k < 12;k ++)
//                          buf[t++] = Net.Buf[35+j+k+1];
//                     break;
//                }
//                j += 13;                         //没有找到，继续查找下一个
//           }
//           i += 4;    // 判断该断电器地址是否还有其它关联传感器,针对于开关量传感器，只计算开关量地址、触发状态两个字节
//           if((addr != Net.Buf[38+allcnt01+i]) || (i == allcnt02))    // 查看下一个，地址不相同，则计算crc
//           {
//                crc = CalCrc16(buf,t,1);
//                Device[addr-1].Crc = crc;
//                t = 0;     //为下一个断电器做准备
//           }
//      }
//      // 最后计算配置信息域（传感器）的crc,13个字节代表一个设备的报警信息域
// CalSensorWarnCrc:
//      for(i = 0;i < allcnt01;)
//      {
//           addr = Net.Buf[35+i];
//           if(Device[addr-1].Flag & 0x04)
//                Net.Buf[35+i+1+1] |= 0x80;
//           crc = CalCrc16(&Net.Buf[35+i],13,1);
//           Device[addr-1].Crc = crc;
//           i += 13;
//      }
// }

void CalDeviceCrc(void)
{
     u8 crc,k,addr,sensoraddr,trigger,buf[255],n;
     u16 i,j,allcnt01,allcnt02,t;
     for(k = 0;k < MaxDeviceNum;k ++)   // ?????????????????
     {
          Device[k].Crc = 0;
          Device[k].CrcCnt = 0;
          Device[k].Flag &= ~0x04;
          Device[k].Warn = 0;
     }
     n = 0;
     for(k = 0;k < MaxTriggerNum;k ++)
     {
          Excute[k].ExcuteAddr = 0;
          Excute[k].TriggerAddr = 0;
          Excute[k].Trigger = 0;
          Excute[k].Warn = 0;
     }
     if(Sys.InitLen == 0x20)    // ???????,???????
          return;
     allcnt01 = Net.Buf[34];
     allcnt01 <<= 8;
     allcnt01 += Net.Buf[33];
     allcnt01 -= 3;                 //?????????????
     if((allcnt01 % 13) != 0)       //1?????13???
          return;
     // ??????????? ???????,?????,??
     if(Sys.InitLen-3 == 32+allcnt01)   //?????????,??
          goto CalSensorWarnCrc;
     allcnt02 = Net.Buf[32+allcnt01+5];
     allcnt02 <<= 8;
     allcnt02 += Net.Buf[32+allcnt01+4];
     allcnt02 -= 3;
     if((allcnt02 % 4) != 0)     //???????,?????????
          goto CalSensorWarnCrc;
     t = 0;
     // ????????  ??????? ????  ??......??
     for(i = 0;i < allcnt02;)
     {
          addr = Net.Buf[38+allcnt01+i];         // ???(??)??
          trigger = Net.Buf[38+allcnt01+i+1];    // ????
          sensoraddr = Net.Buf[38+allcnt01+i+2]; // ?????/?????
          Device[sensoraddr-1].Flag |= 0x10;
          if(trigger & 0x10)               // ????
               Device[sensoraddr-1].Flag |= 0x04;
          Excute[n].TriggerAddr = sensoraddr;
          Excute[n].ExcuteAddr = addr;
          Excute[n].Trigger = trigger;
          n ++;
          if(Net.Buf[38+allcnt01+i+3] == 0x01)   // ???
          {
               Device[sensoraddr-1].Crc |= ((trigger>>3)+1);      // D3 1? D2 0? D1 ??? D0 ????(??D0?????,???????????)
               trigger |= 0x80;
          }
          buf[t++] = sensoraddr;
          buf[t++] = trigger;
          for(j = 0;j < allcnt01;)               // ??????,????????
          {
               if(sensoraddr == Net.Buf[35+j])   //????,??12?????????
               {
                    for(k = 0;k < 12;k ++)
                         buf[t++] = Net.Buf[35+j+k+1];
                    break;
               }
               j += 13;                         //????,???????
          }
          i += 4;    // ???????????????????,?????????,?????????????????
          if((addr != Net.Buf[38+allcnt01+i]) || (i == allcnt02))    // ?????,?????,???crc
          {
               crc = CalCrc16(buf,t,1);
               Device[addr-1].Crc = crc;
               t = 0;     //??????????
          }
     }
     // ?????????(???)?crc,13???????????????
CalSensorWarnCrc:
     for(i = 0;i < allcnt01;)
     {
          addr = Net.Buf[35+i];
          if(Device[addr-1].Flag & 0x04)
               Net.Buf[35+i+1+1] |= 0x80;
          crc = CalCrc16(&Net.Buf[35+i],13,1);
          Device[addr-1].Crc = crc;
          i += 13;
     }
}



void ReadInitInfo(u8 addr)
{
     u16 i,allcnt01,allcnt02,j;
     u8  SensorID,ControlDeviceID,k,trigger;
     
     FramReadBurstByte(BaseInfoAddr,&Net.Buf[0],Sys.InitLen);
     allcnt01 = Net.Buf[34];
     allcnt01 <<= 8;
     allcnt01 += Net.Buf[33];
     allcnt01 -= 3;
// ???????:?? ?? ?? ?? ?? ??
     for(i = 0;i < allcnt01;)
     {
          if(addr == Net.Buf[35+i])   //???????,??????????
          {
               if(Device[addr-1].Flag & 0x04)      //????????
                    Net.Buf[35+i+1+1] |= 0x80;
               for(j = 0;j < 12;j ++)
                    Init.Buf[Init.W ++] = Net.Buf[35+i+1+j];
               return; 
          }
          i += 13;
     }
     //.....????????,?????????????
     allcnt02 = Net.Buf[32+allcnt01+5];
     allcnt02 <<= 8;
     allcnt02 += Net.Buf[32+allcnt01+4];
     allcnt02 -= 3;
     
/* ??????????(???????????????)
 * ??????????14???:???? ?????  ?? ?? ?? ?? ?? ??
 * ??????????2???: ???? ?????
 * ???????????(?????????)
 */
     for(i = 0;i < allcnt02;)
     {
          ControlDeviceID = Net.Buf[38+allcnt01+i];  // ?????
          if(addr != ControlDeviceID)                // ?????????
          {
               i += 4;
               continue;
          }
          SensorID = Net.Buf[38+allcnt01+i+2];  //???????
          Init.Buf[Init.W ++] = SensorID;     
          trigger = Net.Buf[38+allcnt01+i+1];   //???/??? ????
          if(Net.Buf[38+allcnt01+i+3] == 0x01)     // ??????
          {
               trigger |= 0x80;
               Init.Buf[Init.W ++] = trigger;     //????
          }
          else                                     //??????
          {
               Init.Buf[Init.W ++] = trigger;     //????
               for(j = 0;j < allcnt01;)
               {
                    if(SensorID == Net.Buf[35+j])  //???????,???? ???
                    {
                         for(k = 0;k < 12;k ++)
                              Init.Buf[Init.W ++] = Net.Buf[35+j+1+k];
                         break;
                    }
                    j += 13;
               }
          }
          i += 4;
          // ?????,?????,???,?????????????
          // ??????????,???
          if((ControlDeviceID != Net.Buf[38+allcnt01+i]) || (i == allcnt02))
          {
               return;
          }
     }
}

// ???????
void ReadDefaultInit(void)
{
     u16 crc;
     u8 flag;
     flag = 0;
     ReadDefaultInitLen();
     if(Sys.InitLen)
     {
          ReadDefaultInitCrc();
          FramReadBurstByte(BaseInfoAddr,&Net.Buf[0],Sys.InitLen);
          crc = CalCrc16(&Net.Buf[0],Sys.InitLen,0);
          if(crc != Sys.Crc)         //???,??????????
          {
               Sys.Crc = 0;
               flag = 0;
          }
          else            //????????
          {
               flag = 1; 
               ReadDeviceDefine();      //????
               CalDeviceCrc();          //???????crc
          }
     }
     if(flag == 0)          //????????
     {
          ConfigBuf[0] = 0;
          ConfigBuf[1] = 0;
          WriteConfig(InitCrcAddr,2);
          ConfigBuf[0] = 0;
          ConfigBuf[1] = 0;
          WriteConfig(InitLenAddr,2);
     }
}


u8 ReadRecord(u16 addr,u8 *buf)
{
     u8 t[11],i;
     FramReadBurstByte(addr,t,11);
     if(SumCheck(t,11))
     {
          for(i = 0;i < 10;i ++)
               buf[i] = t[i];
          return 1;
     }
     else
          return 0;
}
