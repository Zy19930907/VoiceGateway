#include "IPD_ZJM.h"

_IPD IPDS[MaxIPDNum];

extern CSensor Device[MaxDeviceNum];

u8 CheckIPDIndex(u8 addr)
{
	u8 i;
	for (i = 0; i < MaxPowerNum; i++)
	{
		if (addr == IPDS[i].Addr)
			return i;
	}
	return 0xFF;
}
// �����жϸõ�ַ�Ƿ�������豸�б��У�������ڣ������ж��Ƿ����

u8 FilterIPDAddr(u8 addr)
{
	u8 j;
	if (!(Device[addr - 1].Flag & 0x01))
		return 0xFF;
	for (j = 0; j < MaxIPDNum; j++) //�ٲ��Ҹõ�ַ�Ƿ��Ѿ��ڵ�Դ�б���
	{
		if (addr == IPDS[j].Addr)
			return j;
	}
	// �õ�ַ�����ڣ�����ӵ���������(��ַ�б�Ϊ0�������)
	for (j = 0; j < MaxIPDNum; j++)
	{
		if (IPDS[j].Addr == 0)
			return j;
	}
	return 0xFF;
}

/*
 �ܹ�7֡���ݣ�֡����0----6 
 * ÿһ֡�ĵ�һ���ֽ���֡����+��ֱ��״̬
 */
void WriteIPDRecord(u8 addr, u8 *buf, u32 CanId,u8 len)
{
	u8 i, j,fram;
	if (addr == 0)
		return;
	i = FilterIPDAddr(addr);
	if (i == 0xFF)
		return;
	IPDS[i].Addr = addr;
	fram = (CanId >> 24) & 0x00000000F; //��ȡID�е�֡���
	if(fram == 7)
		Device[addr-1].Buf[0] = buf[3] & 0x01;
	for (j = 0; j < len; j++)
		IPDS[i].UpLoadData[(fram-1) * 8 + j] = buf[j];
}

u32 TimeSwitch(u8 *timebuf)
{
	u32 timeHex;
     timeHex = timebuf[0];    //��
     timeHex <<= 4;
     timeHex += timebuf[1];   //��
     timeHex <<= 5;
     timeHex += timebuf[2];   //��
     timeHex <<= 5;
     timeHex += timebuf[4];   //Сʱ
     timeHex <<= 6;
     timeHex += timebuf[5];   //��
     timeHex <<= 6;
     timeHex += timebuf[6];   //��
	return timeHex;
}

void ReadIPDRecord(u8 *buf,u8 addr)
{
	u8 i,j;
	i = CheckIPDIndex(addr);
	*(buf++) = IPDS[i].Addr;
	*(buf++) = 0x38;
	for(j=0;j<14;j++)
		*(buf++) = IPDS[i].UpLoadData[j+30];
	for(j=0;j<4;j++)
		*(buf++) = IPDS[i].UpLoadData[j+49];
}

u16 ReadDetailIPDRecord(u8 *buf,u8 addr)
{
	u8 i,j,m;
	j = CheckIPDIndex(addr);
	if(j==0xFF)
		j=0;
	for (i = 0; i < 114; i++)
	{
		*(buf++) = IPDS[j].UpLoadData[i];
		m++;
	}
	return m;
}

void ReadAllSetValue(u8 *buf,u8 addr)
{
	u8 i,j;
	i = CheckIPDIndex(addr);
	for(j=0;j<24;j++)
	{
		*(buf+j) = IPDS[i].UpLoadData[j];
	}
}
