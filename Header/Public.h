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
 * �������ش洢��ʽ���£�  ÿ���ֶε����洢 CRC��Ҫռ��2�ֽ�
  ���������Ϣ�洢��оƬ�Դ�������洢�����档�洢��ʽ���£�
 * �洢����              ����       ����λ�ã�EEPROM��
 * ���ص�ַ              3�ֽ�       1,2,3
 * ����IP��ַ��˿ں�    8�ֽ�       4,5,6,7,8,9,10,11
 * �豸����ʱ��          6�ֽ�       12,13,14,15,16,17
 * �豸��������          3�ֽ�       18,19,20
 * ����IP��192.168.1.1�� 6�ֽ�       21,22,23,24,25,26
 * MAC��ַ�洢           8�ֽ�       27,28,29,30,31,32,33,34
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

// ��λ��ÿ125ms����500��ADPCM���� ��1s����4000���ֽ�
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
     RS485Idle,         // ����
     RS485Cycle,        // ��ѯ�㲥
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
     PcToDevice,      // PC�·����������㲥�ն�
     DeviceToPc,      // �㲥�ն˷�����������PC
}CDirection;

typedef struct
{
     CVoiceStatus Status;
     CDirection Dir;
     u8  RecBuf[MaxVoiceLen]; 
     u8  DestAddr;
     u16 R;        // ��ָ��
     u16 W;        // дָ��
     u16 Tick;
	u8 MsCnt;
}CVoice;

/*
 Flag: 
 * D0: �豸��Ч����λ��δ���ã�  0 δ���� 1 �Ѿ�����
 * D1: δע���豸��ͨѶ 0 ����Ϣ 1����Ϣ
 * D2: �豸������߿��� 0 ������   1����
 * D3: ��Ҫͨ��UDP��Ϣת��
 * D4: �������豸�й�����ϵ��1 �й��� 0 �޹���
 * D5: ���ڳ�ʼ�������У�0 �豸���� 1 ���������У���ʱ����������
 */
/*Status   
 * D7 ͨѶ����     0 ����     1 �ж�  
 * D6 D5 ���߹���  00  01 10 11 �ֱ��ʾ1--4����
 * D4 �Լ�����
 * D3......D0 ����״̬��С����
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
    u8  IsAct;//�ô�������ǰ�Ƿ񴥷�����
    u8  Warn;
    u8  RecTimes;
    u16 Tick;    
}CSensor;
/*
 ϵͳFLAG���λ:
 *  D0 ����ʱ��              0:����  1:�쳣
 *  D1 ����λ��ͨѶ�жϱ��  0:����  1:�ж�
 *  D2 ����
 *  D3 socket0 ״̬  0:����   1:�쳣
 *  D4 socket1 ״̬  0:����   1:�쳣
 *  D5 socket2 ״̬  0:����   1:�쳣    ���Թ�������
 *  D6 socket3 ״̬  0:����   1:�쳣    UDPģʽ
 *  D7 ����
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
	u8 InitLeft;       // ��ʼ��ͬ��ʱ��
	u8 SyncLeft;       // ʱ��ͬ��
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
	u8 TriggerAddr;    // ������ַ 
    u8 ExcuteAddr;     // ִ�е�ַ
    u8 Trigger;
    u8 Warn;           // ������־
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

