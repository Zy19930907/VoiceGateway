#ifndef CAN_PRO_H
#define CAN_PRO_H

#include "Public.h"

typedef enum
{
    INIT,
    RX,
    RXOVER,
}CCanStatus;

typedef struct
{
    CCanStatus Status;
    u8  Buf[8];
    u8  Len;
    u16 Tick[4];
    u32 ID;
}CCan;

typedef struct
{
    u8 NeedToNet;
    u8 Addr;
    u16 Tick;
}CAckAddr;

typedef enum
{
    INITIDLE,
    SENDPREPARE,
    SENDING,
}CSendInitStatus;

typedef enum
{
    F_Single,
    F_First,
    F_Middle,
    F_End,
}CFram;

typedef struct
{
    CSendInitStatus Status;
    CFram FramStatus;
    u8  DestAddr;     //?????????  
    u8  W;        // ?
    u8  R;        // ?
    u8  FramCnt;
    u8  Shift;
    u8  Retry;
    u8  Buf[250];
    u16 Tick;
}CSendInit;

typedef struct
{
    u8 Shift;
    u8 Retry;
}CLianDong;

typedef struct
{
	u8 DataL;
	u8 DataH;
	u8 Crc;
	u8 StateName;
}_SwitchSensor;

void CanPro(void);
void CanSendData(u8 index,u32 ID,u8 *buf,u8 len);
u8   SendDataCAN1(u32 ID,u8 *buf,u8 len);
void SendDataCAN23(u8 index,u32 ID,u8 *buf,u8 len);
void CanReceiveFunc(void);
void CanSendInitInfo(void);
void HandleCanData(u32 ID,u8 CanIndex);
void WriteRecord(u8 *buf,u8 len);
void WriteSensorRecord(u8 t,u8 flag);
void TimeChange(u8 *buf);
void AckFunc(u8 index,u8 cmd,u8 DestAddr);
u32 MakeFramID(u8 Fram,u8 Cmd,u8 DestAddr);
void NetInfoToCanOrRs485(u8 cmd,u8 *buf,u8 len);
void WritePowerRecord(u8 addr,u8 *buf);
void ReadCanData(u8 t);
u8 CheckCanRecBuf(void);
u8 MakeCanBusStatus(u8 CanIndex);
u8 CheckPowerIndex(u8 addr);
void SyncTime(u8 CanIndex,u8 addr);
u8 CheckTime(u8 *buf);
void CheckCrc(u8 addr,u8 crc,u8 flag);
void MakeMASWirelessSensor(u8 CanIndex,u8 addr);
void MakeSensorCanData(u32 ID,u8 CanIndex,u8 addr);
void SendLianDong(void);
u8 CheckIPDIndex(u8 addr);
u8 CheckPkPowerIndex(u8 addr);
void Can1RecvFunc(void);
u32 CanIdSwitch(u32 CanId, u8 SwitchDir);
void CheckLianDongInfo(u8 addr,u8 trigger,u8 val);
void SendBreak3_0(void);
void CheckTriggerStatus(void);
#endif

