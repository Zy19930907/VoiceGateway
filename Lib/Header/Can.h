#ifndef CAN_H
#define CAN_H

#include <stm32f10x_lib.h>

typedef enum
{
    INIT,
    RX,
	RXOVER,
}CCanStatus;

typedef struct
{
    CCanStatus Status;
    u8  Buf[8];
    u8  Len;
    u16 Tick[4];
    u32 ID;
}CCan;


void CanInit(void);

#endif
