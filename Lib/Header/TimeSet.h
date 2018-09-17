#ifndef TIME_SET_MENU_H
#define TIME_SET_MENU_H

#include "Public.h"


void DisplayTimeSet(void);
void DisplayOkToReturnTimeSet(void);
void DisplayDiscardTimeSet(void);

void FuncKeyOnTimeSet(void);
void KeyOnTimeSetACK(void);
void FuncKeyOnTimeSetRETURN(void);

void KeyOnTimeSetRETURN(void);
void AddKeyOnTimeSet(u8 keyState);
void DelKeyOnTimeSet(u8 keyState);
void SaveDeviceTime(unsigned char flag);




#endif
