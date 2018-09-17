#ifndef W5500_H
#define W5500_H

#include "Public.h"

/*
 *PB12---------CS
 *PB13---------SCLK
 *PB14---------MISO
 *PB15---------MOSI
 *PC7----------RST
 *PC6----------INT 
 */

#define NET_CS_SET   GPIOB ->ODR |=  0x00001000
#define NET_CS_CLR   GPIOB ->ODR &= ~0x00001000

#define NET_RST_SET  GPIOC ->ODR |=  0x00000080
#define NET_RST_CLR  GPIOC ->ODR &= ~0x00000080

typedef enum
{
    NET_INIT,   //初始化
    NET_LINK,   // 物理连接
    NET_MODE,   // 工作模式
    NET_IDLE,   // 空闲
    NET_INT,    //产生了中断，需要判断进入哪种中断     
    NET_CONNECT,   //连接成功
    NET_RECEIVE,   //接收到数据
    NET_IPCONFLICT, //IP冲突
    NET_UNREACH,    //目标地址未到达
    NET_TIMEOUT,   //超时
}CNetStatus;

/*
 *  D3 socket0 状态  0：正常   1：异常
 *  D4 socket1 状态  0：正常   1：异常
 */ 
typedef struct
{
    CNetStatus Status;
    u8 GatewayIP[4];//网关IP地址 
    u8 SubMask[4];	//子网掩码 
    u8 PhyAddr[6];	//物理地址(MAC) 
    u8 IPAddr[4];	//本机IP地址 

    u8  S0_DestIP[4];	//端口0目的IP地址 
    u8  UdpDipr[4];
    u16 UdpPort;
    u16 SourcePort;  //端口0的端口号
    u16 DestPort;	//端口0目的端口号
    u16 Tick;
    u16 Len;
    u8  Buf[1500];
}CNet;

void W5500IoInit(void);
void W5500Init(void);
u8   SendByte(u8 dat);
void WriteReg1Byte(u16 addr,u8 dat);
void WriteReg2Byte(u16 addr,u16 dat);
void WriteSocket1Byte(u8 index,u16 addr,u8 dat);
void WriteSocket2Byte(u8 index,u16 addr,u16 dat);
void WriteSocket4Byte(u8 index,u16 addr,u8 *buf);
u8 ReadSocket1Byte(u8 index,u16 addr);
u16 ReadSocket2Byte(u8 index,u16 addr);
void ReadSocket4Byte(u8 index,u16 addr,u8 *buf);
u8   ReadReg1Byte(u16 addr);
u16  ReadReg2Byte(u16 addr);
void WriteBurstReg(u16 addr,u8 *buf,u8 len);
void ReadBurstReg(u16 addr,u8 *buf,u8 len);
void SendSocketData(u8 s,u8 *buf,u16 len);
u16 ReadSocketData(u8 s,u8 *buf);
void W5500Reset(void);
void LoadNetParameters(void);
void DetectGateway(void);
u8 Socket_Lisen(u8 s);
u8 SocketConnect(u8 s);
void ClientConect(u8 s);


/***************** Common Register *****************/

#define MR		0x0000
	#define RST		0x80
	#define WOL		0x20
	#define PB		0x10
	#define PPP		0x08
	#define FARP	0x02

#define GAR		0x0001
#define SUBR	0x0005
#define SHAR	0x0009
#define SIPR	0x000f

#define INTLEVEL	0x0013
#define IR		0x0015
	#define CONFLICT	0x80
	#define UNREACH		0x40
	#define PPPOE		0x20
	#define MP			0x10

#define IMR		0x0016
	#define IM_IR7		0x80
	#define IM_IR6		0x40
	#define IM_IR5		0x20
	#define IM_IR4		0x10

#define SIR		0x0017
	#define S7_INT		0x80
	#define S6_INT		0x40
	#define S5_INT		0x20
	#define S4_INT		0x10
	#define S3_INT		0x08
	#define S2_INT		0x04
	#define S1_INT		0x02
	#define S0_INT		0x01

#define SIMR	0x0018
	#define S7_IMR		0x80
	#define S6_IMR		0x40
	#define S5_IMR		0x20
	#define S4_IMR		0x10
	#define S3_IMR		0x08
	#define S2_IMR		0x04
	#define S1_IMR		0x02
	#define S0_IMR		0x01

#define RTR_W5500		0x0019
#define RCR		0x001b

#define PTIMER	0x001c
#define PMAGIC	0x001d
#define PHA		0x001e
#define PSID	0x0024
#define PMRU	0x0026

#define UIPR	0x0028
#define UPORT	0x002c

#define PHYCFGR 0x002e

	#define RST_PHY		0x80
	#define OPMODEW5500		0x40
	#define DPX			0x04
	#define SPD			0x02
	#define LINK		0x01

#define VERR	0x0039

/********************* Socket Register *******************/

#define Sn_MR		0x0000
	#define MULTI_MFEN		0x80
	#define BCASTB			0x40
	#define	ND_MC_MMB		0x20
	#define UCASTB_MIP6B	0x10
	#define MR_CLOSE		0x00
	#define MR_TCP		0x01
	#define MR_UDP		0x02
	#define MR_MACRAW		0x04

#define Sn_CR		0x0001
	#define OPENNET	0x01
	#define LISTEN		0x02
	#define CONNECT	0x04
	#define DISCON		0x08
	#define CLOSENET    0x10
	#define SEND		0x20
	#define SEND_MAC	0x21
	#define SEND_KEEP	0x22
	#define RECV		0x40

#define Sn_IR		0x0002
	#define IR_SEND_OK		0x10
	#define IR_TIMEOUT		0x08
	#define IR_RECV			0x04
	#define IR_DISCON		0x02
	#define IR_CON			0x01

#define Sn_SR		0x0003
	#define SOCK_CLOSED		0x00
	#define SOCK_INIT		0x13
	#define SOCK_LISTEN		0x14
	#define SOCK_ESTABLISHED	0x17
	#define SOCK_CLOSE_WAIT		0x1c
	#define SOCK_UDP		0x22
	#define SOCK_MACRAW		0x02

	#define SOCK_SYNSEND	0x15
	#define SOCK_SYNRECV	0x16
	#define SOCK_FIN_WAI	0x18
	#define SOCK_CLOSING	0x1a
	#define SOCK_TIME_WAIT	0x1b
	#define SOCK_LAST_ACK	0x1d

#define Sn_PORT		0x0004
#define Sn_DHAR	   	0x0006
#define Sn_DIPR		0x000c
#define Sn_DPORTR	0x0010

#define Sn_MSSR		0x0012
#define Sn_TOS		0x0015
#define Sn_TTL		0x0016

#define Sn_RXBUF_SIZE	0x001e
#define Sn_TXBUF_SIZE	0x001f
#define Sn_TX_FSR	0x0020
#define Sn_TX_RD	0x0022
#define Sn_TX_WR	0x0024
#define Sn_RX_RSR	0x0026
#define Sn_RX_RD	0x0028
#define Sn_RX_WR	0x002a

#define Sn_IMR		0x002c
	#define IMR_SENDOK	0x10
	#define IMR_TIMEOUT	0x08
	#define IMR_RECV	0x04
	#define IMR_DISCON	0x02
	#define IMR_CON		0x01

#define Sn_FRAG		0x002d
#define Sn_KPALVTR	0x002f

/*******************************************************************/
/************************ SPI Control Byte *************************/
/*******************************************************************/
/* Operation mode bits */
#define VDM		0x00
#define FDM1	0x01
#define	FDM2	0x02
#define FDM4	0x03

/* Read_Write control bit */
#define RWB_READ	0x00
#define RWB_WRITE	0x04

/* Block select bits */
#define COMMON_R	0x00

/* Socket 0 */
#define S0_REG		0x08
#define S0_TX_BUF	0x10
#define S0_RX_BUF	0x18

/* Socket 1 */
#define S1_REG		0x28
#define S1_TX_BUF	0x30
#define S1_RX_BUF	0x38

/* Socket 2 */
#define S2_REG		0x48
#define S2_TX_BUF	0x50
#define S2_RX_BUF	0x58

/* Socket 3 */
#define S3_REG		0x68
#define S3_TX_BUF	0x70
#define S3_RX_BUF	0x78

/* Socket 4 */
#define S4_REG		0x88
#define S4_TX_BUF	0x90
#define S4_RX_BUF	0x98

/* Socket 5 */
#define S5_REG		0xa8
#define S5_TX_BUF	0xb0
#define S5_RX_BUF	0xb8

/* Socket 6 */
#define S6_REG		0xc8
#define S6_TX_BUF	0xd0
#define S6_RX_BUF	0xd8

/* Socket 7 */
#define S7_REG		0xe8
#define S7_TX_BUF	0xf0
#define S7_RX_BUF	0xf8

#define TRUE	0xff
#define FALSE	0x00

#define S_RX_SIZE	2048	/*定义Socket接收缓冲区的大小，可以根据W5500_RMSR的设置修改 */
#define S_TX_SIZE	2048  	/*定义Socket发送缓冲区的大小，可以根据W5500_TMSR的设置修改 */



#endif

