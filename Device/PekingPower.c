#include "PekingPower.h"

_Pekingower PekingPowers[MaxPkPowerNum];
extern CSensor Device[MaxDeviceNum];
// 查询电源所在的存储编号
u8 CheckPkPowerIndex(u8 addr)
{
	u8 i;
	for (i = 0; i < MaxPowerNum; i++)
	{
		if (addr == PekingPowers[i].Addr)
			return i;
	}
	return 0xFF;
}

// 首先判断该地址是否存在在设备列表中，如果存在，则在判断是否填充
u8 FilterPkPowerAddr(u8 addr)
{
	u8 j;
	if (!(Device[addr - 1].Flag & 0x01))
		return 0xFF;
	for (j = 0; j < MaxPowerNum; j++) //再查找该地址是否已经在电源列表中
	{
		if (addr == PekingPowers[j].Addr)
			return j;
	}
	// 该地址不存在，则添加到缓冲区中(地址列表为0，则填充)
	for (j = 0; j < MaxPowerNum; j++)
	{
		if (PekingPowers[j].Addr == 0)
			return j;
	}
	return 0xFF;
}


void WritePkPowerRecord(u8 addr, u8 *buf,u8 len)
{
	u8 i, j, fram;
	if (addr == 0)
		return;
	i = FilterPkPowerAddr(addr);
	if (i == 0xFF)
		return;
	PekingPowers[i].Addr = addr;
	fram = ((buf[0] >> 3) & 0x07); //获取ID中的帧序号
	if(fram == 0)
	{
		PekingPowers[i].W = 0;
		for (j = 0; j < len; j++)
			PekingPowers[i].BatInfo[PekingPowers[i].W++] = buf[j];
	}
	else
	{
		PekingPowers[i].W = ((fram-1)*6)+8;
		for(j=0;j<len-2;j++)
		{
			PekingPowers[i].BatInfo[PekingPowers[i].W++] = buf[2+j];
			if(PekingPowers[i].W >= 38)
				PekingPowers[i].W = 0;
		}
	}
}
//读取所有电源信息
u16 ReadAllPekingPowerRecord(u8 *buf)
{
	u8 i,j,k=0;
	for(i=0;i<MaxPkPowerNum;i++)
	{
		if(PekingPowers[i].Addr == 0)
			break;
		*(buf++) = PekingPowers[i].Addr;
          for(j = 0;j < 39;j ++)
			*(buf++) = PekingPowers[i].BatInfo[j];
		k+=40;
	}
	return k;
}	
//读取单个电源信息
u16 ReadPekingPowerRecord(u8 *buf,u8 addr)
{
	u8 i,j,k=0;
	*(buf++) = addr;
		j = CheckPkPowerIndex(addr);
		if(j==0xFF)
			j=0;
		for (i = 0; i < 39; i++)
			*(buf++) = PekingPowers[j].BatInfo[i];
		k+= 40;
	return k;
}	
