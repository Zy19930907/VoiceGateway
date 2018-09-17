#ifndef PUBLIC_H
#define PUBLIC_H

#include <stm32f10x_lib.h>


/*
 * 语音网关存储格式如下：  每个字段单独存储 CRC需要占用2字节
  相关配置信息存储在芯片自带的铁电存储器里面。存储格式如下：
 * 存储类型              长度       具体位置（EEPROM）
 * 网关地址              3字节       1,2,3
 * 网关IP地址与端口号    8字节       4,5,6,7,8,9,10,11
 * 设备出厂时间          6字节       12,13,14,15,16,17
 * 设备重启次数          3字节       18,19,20
 * 网关IP（192.168.1.1） 6字节       21,22,23,24,25,26
 * MAC地址存储           8字节       27,28,29,30,31,32,33,34
 */

#define NetAddrAddr             1
#define NetIpAddr               4              
#define FactoryTimeAddr         12
#define ResetTimesAddr          18
#define GateWayIpAddr           21
#define MacBufAddr              27
#define IapServerIPAddr		  35

// 上位机每125ms产生500个ADPCM编码 ，1s产生4000个字节

#define MaxVoiceLen  8000

typedef enum
{
     LedLevel,
     DisplayLevel,
     KeyLevel,
     NetLevel,     
     CanLevel,
	UpdateLevel,
	WarnLevel,
}CRunLevel;

typedef struct
{
    u8 Index;
    u8 UpdateFlag;
    u8 SaveFlag;    
}CMenu;

/*
 系统标志位Flag定义:
 *  D0  是否需要除铁   1: 需要   0：不需要
 *  D1  时候需要报警   1：需要   0：不需要
 *  D2 
 *  D3 
 *  D4 
 */ 
typedef struct
{
    u8 Addr;
    u8 Vol;
    u8 ResetTimes;
    u8 Flag;
    u8 Delay;
    u16 WorkTime;
    u16 AlarmTime;
    u16 ManualCnt;
    u16 AutoCnt;
    u16 NoCnt;
    u16 Crc;           //初始化校验
}CSys;

typedef struct
{
    u8  NeedToNet;
    u8  Addr;
    u16 Tick;
}CAckAddr;

typedef struct
{
    u8 BCDData[7];
    u8 timeHex[7];
}CCurTime;

/*

*/

typedef enum
{
     POWER,
	FRONTCAIJI,
	BEHINDCAIJI,
}CName;

typedef struct
{
    u8 Addr;
    CName Name;
    u8 Status;
    u16 Tick;
}CDevice;

typedef struct
{
     u8  Flag;   //标记
     u8  ChineseIndex;
     u8  X;
     u8  Y;
     u8  Cnt;
     u16 Val;
     u16 Tick;
}CDis;


u16 MsTickDiff(u16 tick);
u16 SecTickDiff(u16 tick);
u8 CRC16Check(u8 *buf,u16 len);
u16 CalCrc16(u8 *buf,u16 len);
void CRC16(u8 value);
void delay_us(u16 nus);
void DelayMs(u16 times);

#endif

