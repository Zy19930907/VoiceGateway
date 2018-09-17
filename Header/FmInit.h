#ifndef FRAMINIT_H
#define FRAMINIT_H

#include "Public.h"

// WP---PC0   SCL----PC1   SDA----PC2
/*
#define FM_SDA_OUT   {GPIOC ->CRL &= 0xFFFFF0FF; GPIOC ->CRL |= 0x00000300;}
#define FM_SDA_IN    {GPIOC ->CRL &= 0xFFFFF0FF; GPIOC ->CRL |= 0x00000800;}

#define FM_SDA_HIGH  GPIOC ->ODR |=  0x00000004
#define FM_SDA_LOW   GPIOC ->ODR &= ~0x00000004

#define FM_SCL_HIGH  GPIOC ->ODR |=  0x00000002
#define FM_SCL_LOW   GPIOC ->ODR &= ~0x00000002

#define FM_SDA_VAL   (GPIOC -> IDR & 0x04)

#define FramWriteEnable   GPIOC ->ODR &= 0xFFFFFFFE
#define FramWriteDisable  GPIOC ->ODR |= 0x00000001   */

#define FM_SDA_OUT   {GPIOA ->CRL &= 0xFFFFFF0F; GPIOA ->CRL |= 0x00000030;}
#define FM_SDA_IN    {GPIOA ->CRL &= 0xFFFFFF0F; GPIOA ->CRL |= 0x00000080;}

#define FM_SDA_HIGH  GPIOA ->ODR |=  0x00000002
#define FM_SDA_LOW   GPIOA ->ODR &= ~0x00000002

#define FM_SCL_HIGH  GPIOA ->ODR |=  0x00000001
#define FM_SCL_LOW   GPIOA ->ODR &= ~0x00000001

#define FM_SDA_VAL   (GPIOA -> IDR & 0x02)

#define FramWriteEnable   GPIOC ->ODR &= 0xFFFFFFF7
#define FramWriteDisable  GPIOC ->ODR |= 0x00000008 


void FmInit(void);
void FramStart(void);
void FramStop(void);
void FramAck(void);
void FramNAck(void);
void FramSendByte(u8 txd);
u8 FramReadByte(void);
u8 FramReadOneByte(u16 Addr);
void FramWriteOneByte(u16 Addr,u8 value);
void FramWriteBurstByte(u16 Addr, u8 *Buf, u16 len);
void FramReadBurstByte(u16 Addr, u8 *Buf, u16 len);


#endif
