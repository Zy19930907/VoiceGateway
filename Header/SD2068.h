/* 
 * File:   SD2068.h
 * Author: dengcongyang
 *
 * Created on 2017年4月7日, 下午12:12
 */

#ifndef   SD2068_H
#define	SD2068_H

#include "Public.h"

/*
SCL       PA6
SDA       PA7
INT       PC4
*/

#define TIM_SDA_OUT        {GPIOA->CRL &=0x0fffffff; GPIOA->CRL |=0x30000000;}
#define TIM_SDA_IN         {GPIOA->CRL &=0x0fffffff; GPIOA->CRL |=0x80000000;}

#define TIM_SDA_HIGH       GPIOA ->ODR |=  0x00000080
#define TIM_SDA_LOW        GPIOA ->ODR &= ~0x00000080

#define TIM_SCL_HIGH       GPIOA ->ODR |=  0x00000040
#define TIM_SCL_LOW        GPIOA ->ODR &= ~0x00000040


#define TIM_SDA_VAL        (GPIOA->IDR &0x00000080)


typedef struct
{
    u8 Buf[7];
    u8 Hex[7];
    u16 AllSecond;
}CTime;

void SD2068Init(void);
void TIM_Start(void);
void TIM_Stop(void);
u8 TIM_Wait_ACK(void);
void TIM_Ack(void);
void TIM_NAck(void);
void TIM_Send_Byte(u8 txd);
u8 TIM_Read_Byte(void);
void ReadTime(void);
void WriteTime(void);
void WriteTimeOn(void);

#endif	/* SD2068_H */

