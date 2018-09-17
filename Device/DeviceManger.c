#include "DeviceManger.h"

_DeviceManger DeviceManger;

//设备管理器函数初始化
void DeviceMangerInit(void)
{
	DeviceManger.WriteRecord_IPD_ZJM = WriteIPDRecord;//初始化写综保记录函数
	DeviceManger.ReadRecord_IPD_ZJM = ReadIPDRecord;//初始化读综保记录函数
	DeviceManger.ReadDetailRecord_IPD_ZJM = ReadDetailIPDRecord;
	
	DeviceManger.WriteRecord_PekingPower = WritePkPowerRecord;
	DeviceManger.ReadAllRecord_PekingPower = ReadAllPekingPowerRecord;
	DeviceManger.ReadRecord_PekingPower = ReadPekingPowerRecord;
	DeviceManger.ReadIPDSetValue = ReadAllSetValue;
	
	DeviceManger.GetSwitcher = FilterSwitcher;
	DeviceManger.SwitcherOnlineCheck = SwitcherOnlineCheck;
	SwitchersInit();
}
