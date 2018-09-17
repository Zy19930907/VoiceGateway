#include "public.h"


#define GENP   0xA001
u16 Crc16;

CSys Sys;
_Break3_0 Breaks3_0;

vu16 SYS_TICK,SYS_TickSec;

void delay_us(u16 nus)
{
     u16 i;
     for(;nus >0;nus --)
         for(i = 0;i < 10;i ++);
}


u16 MsTickDiff(u16 tick)
{
     if(SYS_TICK >= tick)
          return SYS_TICK - tick;
     else
          return 0xffff - tick + SYS_TICK;
}


u16 SecTickDiff(u16 tick)
{
     if(SYS_TickSec >= tick)
          return SYS_TickSec - tick;
     else
          return 0xffff - tick + SYS_TickSec;
}
void CRC16(u8 value)
{
     unsigned char i, temp = 0;
     Crc16 ^= value;
     for(i = 0; i < 8; i ++)
     {
         temp = (Crc16 & 0x0001);
         Crc16 >>= 1;
         Crc16 &= 0x7fff;
         if(temp)
             Crc16 ^= GENP;
     }
}

u16 CalCrc16(u8 *buf,u16 len,u8 flag)
{
     u16 i;
     Crc16 = 0xffff;
     for(i = 0; i < len; i ++)
         CRC16(buf[i]);
     if(flag)
          Crc16 += (Crc16 >> 8);
     return Crc16;
}


u8 CRC16Check(u8 *buf,u16 len)
{
     Crc16 = 0xffff;
     CalCrc16(buf,len-2,0);
     if(((Crc16 & 0x00ff) == buf[len - 2]) && ((Crc16 >> 8) == buf[len - 1]))
         return 0x01;
     else
         return 0x00;
}


u8 SumAll(u8 *buf,u8 len)
{
     u8 i,sum;
     sum = 0;
     for(i = 0;i < len;i ++)
          sum += buf[i];
     return sum;
}

u8 SumCheck(u8 *buf,u8 len)
{
     u8 sum;
     sum = SumAll(buf,len-1);
     if(sum == buf[len-1])
          return 1;
     else
          return 0;
}

void BufCopy(u8 *d, u8 *s, u16 len) 
{
  if (len == 0)
    return;
  do {
    *d++ = *s++;
  } while (--len);
}

void BufClear(u8 *s,u16 len) 
{
  if (len == 0)
    return;
  do {
    *s++ = 0;
  } while (--len);
}

u32 MakeFeimoCanId(u8 FramCnt, u8 Cmd, u8 CtrFlag, u8 Dir, u8 Type, u8 Addr)
{
	u32 FeimoId = 0;
	FeimoId = FramCnt;
	FeimoId <<= 7;
	FeimoId += Cmd;
	FeimoId <<= 2;
	FeimoId += CtrFlag;
	FeimoId <<= 1;
	FeimoId += Dir;
	FeimoId <<= 6;
	FeimoId += Type;
	FeimoId <<= 8;
	FeimoId += Addr;
	return FeimoId;
}

