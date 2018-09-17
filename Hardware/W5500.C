#include <stm32f10x_lib.h>
#include "W5500.h"
#include "sys.h"
#include "config.h"

CNet Net;
extern CSys Sys;
extern vu16  SYS_TICK;
/*
 *PB12---------CS
 *PB13---------SCLK
 *PB14---------MISO
 *PB15---------MOSI
 *PC7----------RST
 *PC6----------INT 
 */

void W5500Reset(void)
{
     u16 tick;
     NET_RST_CLR;
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 200);
     NET_RST_SET;
}

void W5500IoInit(void)
{
// PB12-----CS  PB13------SCLK  PB14-----MISO  PB15-----MOSI
// PC7---RST PC6----INT
 	
	RCC   ->APB2ENR |= 0x08;
	RCC   ->APB1ENR|=1<<14;      //SPI2时钟使能 
	GPIOB ->CRH &= 0X0000FFFF; 
	GPIOB ->CRH |= 0XBBB30000; //PB15 14 13复用 	    
	GPIOB ->ODR |= 0X7<<13;    //PB15 14 13上拉
		
	SPI2->CR1|=0<<10;//全双工模式	
	SPI2->CR1|=1<<9; //软件nss管理
	SPI2->CR1|=1<<8;  

	SPI2->CR1|=1<<2; //SPI主机
	SPI2->CR1|=0<<11;//8bit数据格式	
	SPI2->CR1|=1<<1; //空闲模式下SCK为1 CPOL=1
	SPI2->CR1|=1<<0; //数据采样从第二个时间边沿开始,CPHA=1  
	SPI2->CR1|=1<<3; //Fsck=Fcpu/4   如果为0，则SPI时钟为18M
	SPI2->CR1|=0<<7; //MSBfirst   
	SPI2->CR1|=1<<6; //SPI设备使能
     
     
     RCC   ->APB2ENR |= 0x10;    //PC口
     GPIOC ->CRL &= 0x00FFFFFF;
     GPIOC ->CRL |= 0x38000000;

//	Ex_NVIC_Config(GPIO_C,6,FTIR);//下降沿触发
//	MY_NVIC_Init(2,2,EXTI9_5_IRQChannel,2);//抢占2，子优先级1，组2
}


void LoadNetParameters(void)
{
     u8 i;
     u16 tick;

     Net.SubMask[0] = 255;
     Net.SubMask[1] = 255;
     Net.SubMask[2] = 255;
     Net.SubMask[3] = 0;
  
    // 如果是服务器，则等待目标地址来连接 

     WriteReg1Byte(MR, RST);
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 10);
     WriteBurstReg(GAR,Net.GatewayIP,4);
     WriteBurstReg(SUBR,Net.SubMask,4);
     WriteBurstReg(SHAR,Net.MacAddr,6);
     WriteBurstReg(SIPR,Net.IPAddr,4);

     for(i = 0;i < 8;i ++)
     {
          WriteSocket1Byte(i,Sn_RXBUF_SIZE,0x02);
          WriteSocket1Byte(i,Sn_TXBUF_SIZE,0x02);
     }
     WriteReg2Byte(RTR,2000);    // 0.1MS 为一个单位，重传超时时间
     WriteReg1Byte(RCR,8);       // 重传次数
     WriteReg1Byte(IMR,IM_IR7 | IM_IR6);   //IP地址冲突和目的地址不能到达中断
     WriteReg1Byte(SIMR,S0_IMR | S1_IMR | S2_IMR);
     WriteSocket1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
     WriteSocket1Byte(1, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
     WriteSocket1Byte(2, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
}

void DetectGateway(void)
{
     u16 tick; 
     WriteSocket1Byte(0,Sn_MR,MR_TCP);
     WriteSocket1Byte(0,Sn_CR,OPENNET);      // 打开socket
     
     WriteSocket1Byte(1,Sn_MR,MR_TCP);
     WriteSocket1Byte(1,Sn_CR,OPENNET);      // 打开socket
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     WriteSocket1Byte(0,Sn_CR,CONNECT);   //设置socket为连接模式
     WriteSocket1Byte(1,Sn_CR,CONNECT);   //设置socket为连接模式
}


u8 Socket_Lisen(u8 s)
{
     u8 status;
     u16 tick;
     WriteSocket2Byte(s,Sn_PORT,5000+s);   //设置端口s的源地址
     WriteSocket1Byte(s,Sn_MR,MR_TCP);//设置socket为TCP模式 
     WriteSocket1Byte(s,Sn_CR,OPENNET);//打开Socket	
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     status = ReadSocket1Byte(s,Sn_SR);
     if(status != SOCK_INIT)//如果socket打开失败
     {
          WriteSocket1Byte(s,Sn_CR,CLOSENET);//打开不成功,关闭Socket
          return 0;
     }
     WriteSocket1Byte(s,Sn_CR,LISTEN);//设置Socket为侦听模式	
     return 1;
}

unsigned char Socket_Connect(u8 s)
{
     u16 tick;
     WriteSocket1Byte(s,Sn_MR,MR_TCP);//设置socket为TCP模式
     WriteSocket1Byte(s,Sn_CR,OPENNET);//打开Socket
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     if(ReadSocket1Byte(s,Sn_SR)!=SOCK_INIT)//如果socket打开失败
     {    
          WriteSocket1Byte(s,Sn_CR,CLOSENET);//打开不成功,关闭Socket
          return FALSE;//返回FALSE(0x00)
     }
     WriteSocket1Byte(s,Sn_CR,CONNECT);//设置Socket为Connect模式
     return TRUE;//返回TRUE,设置成功
}

void SendSocketData(u8 s,u8 *buf,u16 len)
{
     u16 offset,offset1,i;
     if(s == 3)
     {
          WriteSocket4Byte(s,Sn_DIPR,Net.UdpDipr);
          WriteSocket2Byte(s,Sn_DPORTR,Net.UdpPort); 
     }
     
     offset=ReadSocket2Byte(s,Sn_TX_WR);
     offset1=offset;
     offset&=(S_TX_SIZE-1);//计算实际的物理地址

     NET_CS_CLR;
     SendByte(offset>>8);//写16位地址
     SendByte(offset);
     SendByte(VDM|RWB_WRITE|(s*0x20+0x10));

     if((offset+len)<S_TX_SIZE)//如果最大地址未超过W5500发送缓冲区寄存器的最大地址
     {
          for(i=0;i<len;i++)
               SendByte(buf[i]);		
     }
     else//如果最大地址超过W5500发送缓冲区寄存器的最大地址
     {
          offset=S_TX_SIZE-offset;
          for(i=0;i<offset;i++)
               SendByte(buf[i]);
          NET_CS_SET;

          NET_CS_CLR;         //地址发生反转
          SendByte(0x00);     //写16位地址
          SendByte(0x00);
          SendByte(VDM|RWB_WRITE|(s*0x20+0x10));
          for(;i<len;i++)
               SendByte(buf[i]);
     }
     NET_CS_SET;
     offset1+=len;//更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
     WriteSocket2Byte(s, Sn_TX_WR, offset1);
     WriteSocket1Byte(s, Sn_CR, SEND);//发送启动发送命令
}

/*
 返回数据缓冲区接收的数据长度
 */

u16 ReadSocketData(u8 s,u8 *buf)
{
     u16 rx_size,offset,offset1,i;
     rx_size = ReadSocket2Byte(s,Sn_RX_RSR);
     
     if(rx_size == 0) 
          return 0;//没接收到数据则返回
     if(rx_size >= 1460) 
          rx_size = 1460;

     offset = ReadSocket2Byte(s,Sn_RX_RD);
     offset1 = offset;
     offset &= (S_RX_SIZE-1);//计算实际的物理地址

     NET_CS_CLR;
     SendByte(offset >> 8);//写16位地址
     SendByte(offset);
     SendByte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
     if((offset + rx_size) < S_RX_SIZE)//如果最大地址未超过W5500接收缓冲区寄存器的最大地址
     {
          for(i = 0;i < rx_size;i ++)
               buf[i] = SendByte(0x00);
     }
     else//如果最大地址超过W5500接收缓冲区寄存器的最大地址
     {
          offset = S_RX_SIZE - offset;
          for(i = 0;i < offset;i ++)
               buf[i] = SendByte(0x00);
          NET_CS_SET;

          NET_CS_CLR;
          SendByte(0x00);//写翻转的地址
          SendByte(0x00);
          SendByte(VDM|RWB_READ|(s*0x20+0x18));
          for(;i < rx_size;i ++)
                buf[i] = SendByte(0x00);
     }
     NET_CS_SET;
     offset1 += rx_size;//更新实际物理地址,即下次读取接收到的数据的起始地址
     WriteSocket2Byte(s, Sn_RX_RD, offset1);
     WriteSocket1Byte(s, Sn_CR, RECV);          //清零接收标志位
     return rx_size;
}


void W5500Init(void)
{
     W5500IoInit();
     W5500Reset();
}


u8   SendByte(u8 dat)
{
     while((SPI2 ->SR & 0x02) == 0);
     SPI2 ->DR = dat;
     while((SPI2 ->SR & 0x01) == 0);
     return SPI2 ->DR;
}

void WriteReg1Byte(u16 addr,u8 dat)
{
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|COMMON_R);
     SendByte(dat);
     NET_CS_SET;
}
void WriteReg2Byte(u16 addr,u16 dat)
{
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|COMMON_R);
     SendByte(dat>>8);
     SendByte(dat);
     NET_CS_SET;
}
void WriteBurstReg(u16 addr,u8 *buf,u8 len)
{
     u8 i;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|COMMON_R);
     for(i = 0;i < len;i ++)
          SendByte(buf[i]);
     NET_CS_SET;
}

u8   ReadReg1Byte(u16 addr)
{
     u8 dat;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_READ|COMMON_R);//通过SPI2写控制字节,1个字节数据长度,读数据,选择通用寄存器
     dat = SendByte(0x00);
     NET_CS_SET;
     return dat;
}

u16 ReadReg2Byte(u16 addr)
{
     u16 dat;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_READ|COMMON_R);//通过SPI2写控制字节,1个字节数据长度,读数据,选择通用寄存器
     dat = SendByte(0x00);
     dat <<= 8;
     dat += SendByte(0x00);
     NET_CS_SET;
     return dat;
}

void ReadBurstReg(u16 addr,u8 *buf,u8 len)
{
     u8 i;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_READ|COMMON_R);//通过SPI2写控制字节,1个字节数据长度,读数据,选择通用寄存器
     for(i = 0;i < len;i ++)
          buf[i] = SendByte(0x00);
     NET_CS_SET;
}

void WriteSocket1Byte(u8 index,u16 addr,u8 dat)
{
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|(index*0x20+0x08));
     SendByte(dat);
     NET_CS_SET;
}
void WriteSocket2Byte(u8 index,u16 addr,u16 dat)
{
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|(index*0x20+0x08));
     SendByte(dat >> 8);
     SendByte(dat);
     NET_CS_SET;
}
void WriteSocket4Byte(u8 index,u16 addr,u8 *buf)
{
     u8 i;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_WRITE|(index*0x20+0x08));
     for(i = 0;i < 4;i ++)
          SendByte(buf[i]);
     NET_CS_SET;
}
u8 ReadSocket1Byte(u8 index,u16 addr)
{
     u8 dat;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_READ|(index*0x20+0x08));
     dat = SendByte(0x00);
     NET_CS_SET;
     return dat;
}

u16 ReadSocket2Byte(u8 index,u16 addr)
{
     u16 dat;
     NET_CS_CLR;
     SendByte(addr >> 8);
     SendByte(addr);
     SendByte(VDM|RWB_READ|(index*0x20+0x08));
     dat = SendByte(0x00);
     dat <<= 8;
     dat += SendByte(0x00);
     NET_CS_SET;
     return dat;
}

void EXTI9_5_IRQHandler(void)
{
	if(!(GPIOC ->IDR & 0x00000040))    // PC6 
		Net.Status = NET_INT;		 
	EXTI->PR = 1<<6;  
}
