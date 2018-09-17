#include "DeviceManger.h"

_DeviceManger DeviceManger;

//�豸������������ʼ��
void DeviceMangerInit(void)
{
	DeviceManger.WriteRecord_IPD_ZJM = WriteIPDRecord;//��ʼ��д�۱���¼����
	DeviceManger.ReadRecord_IPD_ZJM = ReadIPDRecord;//��ʼ�����۱���¼����
	DeviceManger.ReadDetailRecord_IPD_ZJM = ReadDetailIPDRecord;
	
	DeviceManger.WriteRecord_PekingPower = WritePkPowerRecord;
	DeviceManger.ReadAllRecord_PekingPower = ReadAllPekingPowerRecord;
	DeviceManger.ReadRecord_PekingPower = ReadPekingPowerRecord;
	DeviceManger.ReadIPDSetValue = ReadAllSetValue;
	
	DeviceManger.GetSwitcher = FilterSwitcher;
	DeviceManger.SwitcherOnlineCheck = SwitcherOnlineCheck;
	SwitchersInit();
}
