#ifndef PUBLIC_H
#define PUBLIC_H

#include <stm32f10x_lib.h>

#define MaxDmaLen  1010
#define HalfDmaLen 505

#define MaxDeviceNum  128
#define MaxPowerNum   30
#define MaxPkPowerNum 20
#define MaxIPDNum     5
#define NetwayName  30
#define PowerName   0x23
#define SoftVerb    45
#define MaxTriggerNum  200
#define FramBegin   2045
#define FramEnd     32768
#define MaxOneFrame 660
#define ACK             0x10
#define PREPARE         0xEC
#define WRONGACK        0xED
#define UnRegister      0x50
#define TimeShift       120

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
#define ReadPointerAddr         27
#define WritePointerAddr        31
#define InitInfoAddr            35
#define InitLenAddr             43
#define InitCrcAddr             47
#define MacBufAddr              51
#define FilterTimeAddr          59
#define BootLoaderAddr          63
#define NetCrcCntAddr           71
#define NetUdpIpAddr            74

#define BaseInfoAddr            128

// 上位机每125ms产生500个ADPCM编码 ，1s产生4000个字节
#define MaxVoiceLen  20000

#define DuanDianQi     0x1F

typedef enum
{
     LedLevel,
     CanLevel,
     InitLevel,
     LianDongLevel,
     VoiceLevel,
     NetLevel,
     Rs485Level,
     UpdateDevice,
}CRunLevel;

typedef enum
{
     RS485Idle,         // 空闲
     RS485Cycle,        // 轮询广播
     RS485SendStep1,
     RS485SendStep2,
     RS485SendStep3,
     RS485Receive,
}CRs485Status;

typedef struct
{
     CRs485Status Status;
     u8  	CycleIndex;
	u8 	CmdBuf[10];
     u8  	Buf[MaxDmaLen];
     u16 	Cnt;
     u16 Tick;
}CRs485;

typedef enum
{
     Idle,
     Send,
     Over,
}CVoiceStatus;

typedef enum
{
     PcToDevice,      // PC下发语音包到广播终端
     DeviceToPc,      // 广播终端发送语音包到PC
}CDirection;

typedef struct
{
     CVoiceStatus Status;
     CDirection Dir;
     u8  RecBuf[MaxVoiceLen]; 
     u8  DestAddr;
     u16 R;        // 读指针
     u16 W;        // 写指针
     u16 Tick;
	u8 MsCnt;
}CVoice;

/*
 Flag: 
 * D0: 设备有效（上位机未配置）  0 未配置 1 已经配置
 * D1: 未注册设备有通讯 0 无信息 1有信息
 * D2: 设备参与断线控制 0 不参与   1参与
 * D3: 需要通过UDP信息转发
 * D4: 和其它设备有关联关系：1 有关联 0 无关联
 * D5: 处于初始化配置中：0 设备正常 1 处于配置中，暂时不触发动作
 */
/*Status   
 * D7 通讯正常     0 正常     1 中断  
 * D6 D5 总线归属  00  01 10 11 分别表示1--4总线
 * D4 自检数据
 * D3......D0 数据状态与小数点
 */
typedef struct
{
    u8  Flag;
    u8  Status;
    u8  Name;
    u8  Buf[3];
    u8  Crc;
    u8  CrcCnt;
    u8  CheckCnt;
    u8  IsAct;//该传感器当前是否触发联动
    u8  Warn;
    u8  RecTimes;
    u16 Tick;    
}CSensor;
/*
 系统FLAG标记位:
 *  D0 网关时间              0:正常  1:异常
 *  D1 与上位机通讯中断标记  0:正常  1:中断
 *  D2 保留
 *  D3 socket0 状态  0:正常   1:异常
 *  D4 socket1 状态  0:正常   1:异常
 *  D5 socket2 状态  0:正常   1:异常    测试工具连接
 *  D6 socket3 状态  0:正常   1:异常    UDP模式
 *  D7 保留
 */
typedef struct
{
	u8 VoiceSocket;
	u8 DebugSocket;
	u8 Addr;
	u8 Vol;
	u8 ResetTimes;
	u8 Flag;
	u8 Delay;
	u8 CrcCnt;
	u8 InitLeft;       // 初始化同步时间
	u8 SyncLeft;       // 时间同步
	u8 BroadCastLeft;
	u8 PauseVoice;
	u8 LianDongCnt;
	u16 InitLen;       //?????
	u16 Crc;           //?????
	u16 R;              //?????
	u16 W;             //?????
	u16 Shift;         //???????
	u16 PauseSec;
	u16 FilterTime;
	u16 ActFilterTime;
}CSys;

typedef struct
{
	u8 Addr;
	u8 BatInfo[46];
}CPower;


typedef struct
{
	u8 TriggerAddr;    // 触发地址 
    u8 ExcuteAddr;     // 执行地址
    u8 Trigger;
    u8 Warn;           // 报警标志
}CExcute;

typedef struct
{
	u8 BreakerAddr[20];
	u8 CH4Addr[20];
	u8 Cnt;
}_Break3_0;


extern _Break3_0 Breaks3_0;

#define NetAttri    0x1E

#define DEVICERESET 1
#define SENSORVARY  2
#define CARDVARY    3
#define NETRESET    4
#define LianDongRecord   5
#define NEWCARDVARY  6

u16 MsTickDiff(u16 tick);
u16 SecTickDiff(u16 tick);
void delay_us(u16 time);
void CRC16(u8 value);
u16 CalCrc16(u8 *buf,u16 len,u8 flag);
u8 CRC16Check(u8 *buf,u16 len);
u8 SumAll(u8 *buf,u8 len);
u8 SumCheck(u8 *buf,u8 len);
void BufCopy(u8 *d, u8 *s, u16 len);
void BufClear(u8 *s,u16 len) ;
u32 MakeFeimoCanId(u8 FramCnt, u8 Cmd, u8 CtrFlag, u8 Dir, u8 Type, u8 Addr);
#endif

