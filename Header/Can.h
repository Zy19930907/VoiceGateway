#ifndef CAN_H
#define CAN_H

#include <stm32f10x_lib.h>


/*
 * 第二路CAN
 * PB11---CS  PC10---MISO   PC11---MOSI   PC12---SCLK   
 * PD2---INT 
 */
#define SET_SCLK_2     GPIOC ->ODR |=  0x00001000
#define CLR_SCLK_2     GPIOC ->ODR &= ~0x00001000
#define SET_MOSI_2     GPIOC ->ODR |=  0x00000800 
#define CLR_MOSI_2     GPIOC ->ODR &= ~0x00000800
#define SET_CS_2       GPIOB ->ODR |=  0x00000800
#define CLR_CS_2       GPIOB ->ODR &= ~0x00000800
#define MISO_2        ((GPIOC ->IDR &  0x00000400)>>8)
/*
 * 第三路CAN
 * PB5---CS  PB6---MISO   PB7---MOSI   PB8---SCLK   
 * PB9---INT 
 */
#define SET_SCLK_3     GPIOB ->ODR |=  0x00000100
#define CLR_SCLK_3     GPIOB ->ODR &= ~0x00000100
#define SET_MOSI_3     GPIOB ->ODR |=  0x00000080
#define CLR_MOSI_3     GPIOB ->ODR &= ~0x00000080
#define SET_CS_3       GPIOB ->ODR |=  0x00000020
#define CLR_CS_3       GPIOB ->ODR &= ~0x00000020
#define MISO_3        (GPIOB ->IDR &   0x00000040)

#define CANIDSWITCH_B2F                                  0x00
#define CANIDSWITCH_F2B                                  0x01

void Can1Init(void);
void InitDMACan(void);
void Can2Init(void);
void Can3Init(void);
void MCP2515Reset(u8 index);
u8  SendByteCan(u8 index,u8 val);
u8 ReadStatus(u8 index);
void ModifyReg(u8 index,u8 addr,u8 mask,u8 val);
void ReadBurstRegCan(u8 index,u8 addr,u8 *buf,u8 len);
u8 ReadRegCan(u8 index,u8 addr);
void WriteBurstRegCan(u8 index,u8 addr, u8 *buf, u8 len);
void WriteRegCan(u8 index,u8 addr, u8 value);
void Mcp2515Init(u8 index);
u8 GetMISO(u8 t);
void SET_CS(u8 t);
void CLR_CS(u8 t);
void SET_MOSI(u8 t);
void CLR_MOSI(u8 t);
void SET_SCLK(u8 t);
void CLR_SCLK(u8 t);

/*
 ??????
 */
#define NORMAL_MODE    0x00
#define SLEEP_MODE     0x20
#define LOOPBACK_MODE  0x40
#define LISTEN_MODE    0x60
#define CONFIG_MODE    0x80


/* Configuration Registers */
#define CANSTAT         0x0E
#define CANCTRL         0x0F
#define BFPCTRL         0x0C
#define TEC             0x1C
#define REC             0x1D
#define CNF3            0x28
#define CNF2            0x29
#define CNF1            0x2A
#define CANINTE         0x2B
#define CANINTF         0x2C
#define EFLG            0x2D
#define TXRTSCTRL       0x0D

/*  Recieve Filters */
#define RXF0SIDH        0x00
#define RXF0SIDL        0x01
#define RXF0EIDH        0x02
#define RXF0EIDL        0x03
#define RXF1SIDH        0x04
#define RXF1SIDL        0x05
#define RXF1EID8        0x06
#define RXF1EID0        0x07
#define RXF2SIDH        0x08
#define RXF2SIDL        0x09
#define RXF2EID8        0x0A
#define RXF2EID0        0x0B
#define RXF3SIDH        0x10
#define RXF3SIDL        0x11
#define RXF3EID8        0x12
#define RXF3EID0        0x13
#define RXF4SIDH        0x14
#define RXF4SIDL        0x15
#define RXF4EID8        0x16
#define RXF4EID0        0x17
#define RXF5SIDH        0x18
#define RXF5SIDL        0x19
#define RXF5EID8        0x1A
#define RXF5EID0        0x1B

/* Receive Masks */
#define RXM0SIDH        0x20
#define RXM0SIDL        0x21
#define RXM0EIDH        0x22
#define RXM0EIDL        0x23
#define RXM1SIDH        0x24
#define RXM1SIDL        0x25
#define RXM1EID8        0x26
#define RXM1EID0        0x27

/* Tx Buffer 0 */
#define TXB0CTRL        0x30
#define TXB0SIDH        0x31
#define TXB0SIDL        0x32
//#define TXB0EID8        0x33    //??????TXB0EID8,???? TXB0EIDH
//#define TXB0EID0        0x34
#define TXB0EIDH        0x33
#define TXB0EIDL        0x34
#define TXB0DLC         0x35
#define TXB0D0          0x36
#define TXB0D1          0x37
#define TXB0D2          0x38
#define TXB0D3          0x39
#define TXB0D4          0x3A
#define TXB0D5          0x3B
#define TXB0D6          0x3C
#define TXB0D7          0x3D
                         
/* Tx Buffer 1 */
#define TXB1CTRL        0x40
#define TXB1SIDH        0x41
#define TXB1SIDL        0x42
#define TXB1EID8        0x43
#define TXB1EID0        0x44
#define TXB1DLC         0x45
#define TXB1D0          0x46
#define TXB1D1          0x47
#define TXB1D2          0x48
#define TXB1D3          0x49
#define TXB1D4          0x4A
#define TXB1D5          0x4B
#define TXB1D6          0x4C
#define TXB1D7          0x4D

/* Tx Buffer 2 */
#define TXB2CTRL        0x50
#define TXB2SIDH        0x51
#define TXB2SIDL        0x52
#define TXB2EID8        0x53
#define TXB2EID0        0x54
#define TXB2DLC         0x55
#define TXB2D0          0x56
#define TXB2D1          0x57
#define TXB2D2          0x58
#define TXB2D3          0x59
#define TXB2D4          0x5A
#define TXB2D5          0x5B
#define TXB2D6          0x5C
#define TXB2D7          0x5D
                         
/* Rx Buffer 0 */
#define RXB0CTRL        0x60
#define RXB0SIDH        0x61
#define RXB0SIDL        0x62
//#define RXB0EID8        0x63
//#define RXB0EID0        0x64
#define RXB0EIDH        0x63
#define RXB0EIDL        0x64
#define RXB0DLC         0x65
#define RXB0D0          0x66
#define RXB0D1          0x67
#define RXB0D2          0x68
#define RXB0D3          0x69
#define RXB0D4          0x6A
#define RXB0D5          0x6B
#define RXB0D6          0x6C
#define RXB0D7          0x6D
                         
/* Rx Buffer 1 */
#define RXB1CTRL        0x70
#define RXB1SIDH        0x71
#define RXB1SIDL        0x72
//#define RXB1EID8        0x73
//#define RXB1EID0        0x74
#define RXB1EIDH        0x73
#define RXB1EIDL        0x74
#define RXB1DLC         0x75
#define RXB1D0          0x76
#define RXB1D1          0x77
#define RXB1D2          0x78
#define RXB1D3          0x79
#define RXB1D4          0x7A
#define RXB1D5          0x7B
#define RXB1D6          0x7C
#define RXB1D7          0x7D

//?????BFPCTRL???
#define B1BFS  5
#define B0BFS  4
#define B1BFE  3
#define B0BFE  2
#define B1BFM  1
#define B0BFM  0

//?????TXRTSCTRL???
#define B2RTS  5
#define B1RTS  4
#define B0RTS  3
#define B2RTSM  2
#define B1RTSM  1
#define B0RTSM  0

//?????CANSTAT???
#define OPMOD2  7
#define OPMOD1  6
#define OPMOD0  5
#define ICOD2  3
#define ICOD1  2
#define ICOD0  1


//?????CANCTRL???
#define REQOP2  7
#define REQOP1  6
#define REQOP0  5
#define ABAT  4
#define CLKEN  2
#define CLKPRE1  1
#define CLKPRE0  0


 //?????CNF3???
#define WAKFIL  6
#define PHSEG22  2
#define PHSEG21  1
#define PHSEG20  0


 //?????CNF2???
#define BTLMODE  7
//#define SAM   6
#define PHSEG12  5
#define PHSEG11  4
#define PHSEG10  3
#define PHSEG2  2
#define PHSEG1  1
#define PHSEG0  0


 //?????CNF1???
#define SJW1  7
#define SJW0  6
#define BRP5  5
#define BRP4  4
#define BRP3  3
#define BRP2  2
#define BRP1  1
#define BRP0  0


 //?????CANINTE???
#define MERRE  7
#define WAKIE  6
#define ERRIE  5
#define TX2IE  4
#define TX1IE  3
#define TX0IE  2
#define RX1IE  1
#define RX0IE  0


 //?????CANINTF???
#define MERRF  7
#define WAKIF  6
#define ERRIF  5
#define TX2IF  4
#define TX1IF  3
#define TX0IF  2
#define RX1IF  1
#define RX0IF  0


 //?????EFLG???
#define RX1OVR  7
#define RX0OVR  6
#define TXB0  5
#define TXEP  4
#define RXEP  3
#define TXWAR  2
#define RXWAR  1
#define EWARN  0


 //?????TXBnCTRL ( n = 0, 1, 2 )???
#define ABTF  6
#define MLOA  5
#define TXERR  4
#define TXREQ  3
#define TXP1  1
#define TXP0  0


 //?????RXB0CTRL???
#define RXM1  6
#define RXM0  5
#define RXRTR  3
#define BUKT  2
#define BUKT1  1
#define FILHIT0  0

//????????? TXBnSIDL ( n = 0, 1 )????


//???????1?????????
#define FILHIT2  2
#define FILHIT1  1

/**
 * ???????n??????? RXBnSIDL ( n = 0, 1 )????
 */
#define SRR   4


// ???????n????? RXBnDLC ( n = 0, 1 )????

#define DLC3  3
#define DLC2  2
#define DLC1  1
#define DLC0  0


#define CAN_RESET       0xC0
#define CAN_WRITE       0x02
#define CAN_READ        0x03
#define CAN_RTS         0x80    // ????
#define CAN_RTS_TXB0    0x81    // ???????1
#define CAN_RTS_TXB1    0x82    // ???????2
#define CAN_RTS_TXB2    0x84    // ???????3
#define CAN_RD_STATUS   0xA0    // ?????
#define CAN_BIT_MODIFY  0x05    // ????????
#define CAN_RX_STATUS   0xB0    // ??????
#define CAN_RD_RX_BUFF  0x90    // ????????
#define CAN_LOAD_TX     0X40    // ???????
#endif	/* CAN_H */


