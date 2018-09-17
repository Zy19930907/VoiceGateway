#ifndef __IPD_ZJM_H
#define __IPD_ZJM_H

#include "Public.h"

typedef struct
{
    u8 Addr;
    u8 Status;
    u8 UpLoadData[114];
}_IPD;
void WriteIPDRecord(u8 addr, u8 *buf, u32 CanId,u8 len);
u8 CheckIPDIndex(u8 addr);
u8 FilterIPDAddr(u8 addr);
void ReadIPDRecord(u8 *buf,u8 addr);
u16 ReadDetailIPDRecord(u8 *buf,u8 addr);
void ReadAllSetValue(u8 *buf,u8 addr);

#endif
