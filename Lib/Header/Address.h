#ifndef ADDRESS_SET_H
#define ADDRESS_SET_H

#include "Public.h"


void SaveDeviceAddress(u8 flag);

void FuncKeyOnAddressSet(void);
void FuncKeyOnAddressSetAck(void);
void FuncKeyOnAddressSetReturn(void);

void DisplayAddressSet(void);
void DisplayAddressSetAck(void);
void DisplayAddressSetReturn(void);

void ReadDefaultAddress(void);

void AddKeyOnAddressSet(u8 keyState);
void DelKeyOnAddressSet(u8 keyState);

void KeyOnAddressSetReturn(void);


#endif

