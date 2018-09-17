#include "Switcher.h"
_Switcher Switchers[MaxSwitcher];
_Switcher NoSwitcher,NullSwitcher;
extern CSensor Device[MaxDeviceNum];

void SwitchersInit(void)
{
	u8 i;
	//转换板偏移地址可能为0，使用0xFE标记该域未使用
	for(i=0;i<MaxSwitcher;i++)
		Switchers[i].Addr = 0xFE;
}


u8 FilterSwitcher(u8 Addr)
{
	u8 i;
	for(i=0;i<MaxSwitcher;i++)
	{
		if(Switchers[i].Addr == Addr)
			return i;
	}
	for(i=0;i<MaxSwitcher;i++)
	{
		if(Switchers[i].Addr == 0xFE)
			return i;
	}
	return 0xFF;
}

void SwitcherOnlineCheck(void)
{
	u8 i,j,index;
	for(i=0;i<MaxSwitcher;i++)
	{
		//转换板已启用且10秒未收到该转换板数据则认为该转换板下所有传感器断线
		if((Switchers[i].Flag & 0x80) && (MsTickDiff(Switchers[i].Tick) >= 10000))
		{
			for(j=0;j<16;j++)//一个转换板管理16个传感器
			{
				index = Switchers[i].Addr+j;
				Device[index].Status |= 0x80;
				Device[index].Flag &= ~0x02;
			}
			Switchers[i].Flag = 0;
			Switchers[i].Addr = 0xFE;
			Switchers[i].Tick = 0;
		}
	}
}
