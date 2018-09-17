#ifndef LCD_H
#define LCD_H

#include "Public.h"

/*
// PC0-3 对应LCD 的 RS WR RD CS1 
// PD2---RST_LCD
// PC10--12 数据位 DB0-2  PB5--PB9 数据位 DB3--7
可以显示8行，每行15个字

RS   PC0
WR   PC1
RD   PC2
CS1  PC3
RST  PD2
DBx  PC10---PC12  PB5---PB9
*/

void LCD_Init(void);
void LcdIoInit(void);
void LCD_SoftRst(void);
void LCD_CmdWrite(unsigned char cmdaddr);
void LCD_DataWrite(unsigned char WrData);
void LCD_Config_Inital(void);
void ClearSnow(void);
void LcdClear(void);
void LCD_Graphic(void);
void GotoXY(unsigned char xx,unsigned char yy);
unsigned char LCD_CmdRead(unsigned char Addr);
void NoClearSnow(void);
void LCD_Text(void);
void Display(unsigned char *text);
void DisplayNum(u8 X,u8 Y,u16 val);
void DisplayBlack(unsigned char xx,unsigned char yy,unsigned char count);
void UpdateTimeFunc(void);
void ShowCursor(u8 X, u8 Y);
void CleanCursor(u8 X, u8 Y);


//行显示，行与行之间步进16， 而列，同行的时候，字符占用1个区间，而汉字占用2个区间
#define FIRST_ROW     0
#define SECOND_ROW    16
#define THIRD_ROW     32
#define FOUTH_ROW     48
#define FIFTH_ROW     64
#define SIXTH_ROW     80
#define SEVENTH_ROW   96
#define EIGTH_ROW     112

/*
RS   PC0
WR   PC1
RD   PC2
CS1  PC3
RST  PD2
*/
#define RS_SET   GPIOC ->ODR |= 0x00000001
#define RS_CLR   GPIOC ->ODR &= ~0x00000001
#define WR_SET   GPIOC ->ODR |= 0x00000002
#define WR_CLR   GPIOC ->ODR &= ~0x00000002
#define RD_SET   GPIOC ->ODR |= 0x00000004
#define RD_CLR   GPIOC ->ODR &= ~0x00000004
#define CS_SET   GPIOC ->ODR |= 0x00000008
#define CS_CLR   GPIOC ->ODR &= ~0x00000008
#define RST_SET  GPIOD ->ODR |= 0x00000004
#define RST_CLR  GPIOD ->ODR &= ~0x00000004



#define WLCR	0x00
#define MISC	0x01
#define ADSR	0x03
#define INTR	0x0F
#define WCCR	0x10
#define CHWI   0x11						//new(have change)
#define MAMR	0x12
#define AWRR	0x20
#define DWWR	0x21
#define AWBR	0x30
#define DWHR	0x31						//new(have change)
#define AWLR	0x40
#define AWTR	0x50
#define CURX	0x60						//new(have change)
#define BGSG	0x61
#define EDSG    0x62						//new
#define CURY	0x70						//new(have change)	
#define BGCM	0x71
#define EDCM	0x72
#define BTMR	0x80
#define ITCR	0x90						//new(have change)
#define KSCR1	0xA0
#define KSCR2	0xA1
#define KSDR0  0xA2						//new(have change)
#define KSDR1  0xA3
#define KSDR2  0xA4
#define MWCR	0xB0						//new(have change)
#define MRCR	0xB1						//new
#define TPCR	0xC0
#define TPXR	0xC1
#define TPYR	0xC2
#define TPZR	0xC3
#define PCR    0xD0						//new(have change)
#define PDCR   0xD1						//new
#define PNTR	0xE0
#define FNCR	0xF0
#define FVHT	0xF1



#endif
