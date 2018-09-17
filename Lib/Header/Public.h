#ifndef PUBLIC_H
#define PUBLIC_H

#include <stm32f10x_lib.h>


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
#define MacBufAddr              27
#define IapServerIPAddr		  35

// ��λ��ÿ125ms����500��ADPCM���� ��1s����4000���ֽ�

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
 ϵͳ��־λFlag����:
 *  D0  �Ƿ���Ҫ����   1: ��Ҫ   0������Ҫ
 *  D1  ʱ����Ҫ����   1����Ҫ   0������Ҫ
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
    u16 Crc;           //��ʼ��У��
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
     u8  Flag;   //���
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

