#include "Switcher.h"
_Switcher Switchers[MaxSwitcher];
_Switcher NoSwitcher,NullSwitcher;
extern CSensor Device[MaxDeviceNum];

void SwitchersInit(void)
{
	u8 i;
	//ת����ƫ�Ƶ�ַ����Ϊ0��ʹ��0xFE��Ǹ���δʹ��
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
		//ת������������10��δ�յ���ת������������Ϊ��ת���������д���������
		if((Switchers[i].Flag & 0x80) && (MsTickDiff(Switchers[i].Tick) >= 10000))
		{
			for(j=0;j<16;j++)//һ��ת�������16��������
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
