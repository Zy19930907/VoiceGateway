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
	RCC   ->APB1ENR|=1<<14;      //SPI2ʱ��ʹ�� 
	GPIOB ->CRH &= 0X0000FFFF; 
	GPIOB ->CRH |= 0XBBB30000; //PB15 14 13���� 	    
	GPIOB ->ODR |= 0X7<<13;    //PB15 14 13����
		
	SPI2->CR1|=0<<10;//ȫ˫��ģʽ	
	SPI2->CR1|=1<<9; //����nss����
	SPI2->CR1|=1<<8;  

	SPI2->CR1|=1<<2; //SPI����
	SPI2->CR1|=0<<11;//8bit���ݸ�ʽ	
	SPI2->CR1|=1<<1; //����ģʽ��SCKΪ1 CPOL=1
	SPI2->CR1|=1<<0; //���ݲ����ӵڶ���ʱ����ؿ�ʼ,CPHA=1  
	SPI2->CR1|=1<<3; //Fsck=Fcpu/4   ���Ϊ0����SPIʱ��Ϊ18M
	SPI2->CR1|=0<<7; //MSBfirst   
	SPI2->CR1|=1<<6; //SPI�豸ʹ��
     
     
     RCC   ->APB2ENR |= 0x10;    //PC��
     GPIOC ->CRL &= 0x00FFFFFF;
     GPIOC ->CRL |= 0x38000000;

//	Ex_NVIC_Config(GPIO_C,6,FTIR);//�½��ش���
//	MY_NVIC_Init(2,2,EXTI9_5_IRQChannel,2);//��ռ2�������ȼ�1����2
}


void LoadNetParameters(void)
{
     u8 i;
     u16 tick;

     Net.SubMask[0] = 255;
     Net.SubMask[1] = 255;
     Net.SubMask[2] = 255;
     Net.SubMask[3] = 0;
  
    // ����Ƿ���������ȴ�Ŀ���ַ������ 

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
     WriteReg2Byte(RTR,2000);    // 0.1MS Ϊһ����λ���ش���ʱʱ��
     WriteReg1Byte(RCR,8);       // �ش�����
     WriteReg1Byte(IMR,IM_IR7 | IM_IR6);   //IP��ַ��ͻ��Ŀ�ĵ�ַ���ܵ����ж�
     WriteReg1Byte(SIMR,S0_IMR | S1_IMR | S2_IMR);
     WriteSocket1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
     WriteSocket1Byte(1, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
     WriteSocket1Byte(2, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
}

void DetectGateway(void)
{
     u16 tick; 
     WriteSocket1Byte(0,Sn_MR,MR_TCP);
     WriteSocket1Byte(0,Sn_CR,OPENNET);      // ��socket
     
     WriteSocket1Byte(1,Sn_MR,MR_TCP);
     WriteSocket1Byte(1,Sn_CR,OPENNET);      // ��socket
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     WriteSocket1Byte(0,Sn_CR,CONNECT);   //����socketΪ����ģʽ
     WriteSocket1Byte(1,Sn_CR,CONNECT);   //����socketΪ����ģʽ
}


u8 Socket_Lisen(u8 s)
{
     u8 status;
     u16 tick;
     WriteSocket2Byte(s,Sn_PORT,5000+s);   //���ö˿�s��Դ��ַ
     WriteSocket1Byte(s,Sn_MR,MR_TCP);//����socketΪTCPģʽ 
     WriteSocket1Byte(s,Sn_CR,OPENNET);//��Socket	
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     status = ReadSocket1Byte(s,Sn_SR);
     if(status != SOCK_INIT)//���socket��ʧ��
     {
          WriteSocket1Byte(s,Sn_CR,CLOSENET);//�򿪲��ɹ�,�ر�Socket
          return 0;
     }
     WriteSocket1Byte(s,Sn_CR,LISTEN);//����SocketΪ����ģʽ	
     return 1;
}

unsigned char Socket_Connect(u8 s)
{
     u16 tick;
     WriteSocket1Byte(s,Sn_MR,MR_TCP);//����socketΪTCPģʽ
     WriteSocket1Byte(s,Sn_CR,OPENNET);//��Socket
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
     if(ReadSocket1Byte(s,Sn_SR)!=SOCK_INIT)//���socket��ʧ��
     {    
          WriteSocket1Byte(s,Sn_CR,CLOSENET);//�򿪲��ɹ�,�ر�Socket
          return FALSE;//����FALSE(0x00)
     }
     WriteSocket1Byte(s,Sn_CR,CONNECT);//����SocketΪConnectģʽ
     return TRUE;//����TRUE,���óɹ�
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
     offset&=(S_TX_SIZE-1);//����ʵ�ʵ�������ַ

     NET_CS_CLR;
     SendByte(offset>>8);//д16λ��ַ
     SendByte(offset);
     SendByte(VDM|RWB_WRITE|(s*0x20+0x10));

     if((offset+len)<S_TX_SIZE)//�������ַδ����W5500���ͻ������Ĵ���������ַ
     {
          for(i=0;i<len;i++)
               SendByte(buf[i]);		
     }
     else//�������ַ����W5500���ͻ������Ĵ���������ַ
     {
          offset=S_TX_SIZE-offset;
          for(i=0;i<offset;i++)
               SendByte(buf[i]);
          NET_CS_SET;

          NET_CS_CLR;         //��ַ������ת
          SendByte(0x00);     //д16λ��ַ
          SendByte(0x00);
          SendByte(VDM|RWB_WRITE|(s*0x20+0x10));
          for(;i<len;i++)
               SendByte(buf[i]);
     }
     NET_CS_SET;
     offset1+=len;//����ʵ��������ַ,���´�д���������ݵ��������ݻ���������ʼ��ַ
     WriteSocket2Byte(s, Sn_TX_WR, offset1);
     WriteSocket1Byte(s, Sn_CR, SEND);//����������������
}

/*
 �������ݻ��������յ����ݳ���
 */

u16 ReadSocketData(u8 s,u8 *buf)
{
     u16 rx_size,offset,offset1,i;
     rx_size = ReadSocket2Byte(s,Sn_RX_RSR);
     
     if(rx_size == 0) 
          return 0;//û���յ������򷵻�
     if(rx_size >= 1460) 
          rx_size = 1460;

     offset = ReadSocket2Byte(s,Sn_RX_RD);
     offset1 = offset;
     offset &= (S_RX_SIZE-1);//����ʵ�ʵ�������ַ

     NET_CS_CLR;
     SendByte(offset >> 8);//д16λ��ַ
     SendByte(offset);
     SendByte(VDM|RWB_READ|(s*0x20+0x18));//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���
     if((offset + rx_size) < S_RX_SIZE)//�������ַδ����W5500���ջ������Ĵ���������ַ
     {
          for(i = 0;i < rx_size;i ++)
               buf[i] = SendByte(0x00);
     }
     else//�������ַ����W5500���ջ������Ĵ���������ַ
     {
          offset = S_RX_SIZE - offset;
          for(i = 0;i < offset;i ++)
               buf[i] = SendByte(0x00);
          NET_CS_SET;

          NET_CS_CLR;
          SendByte(0x00);//д��ת�ĵ�ַ
          SendByte(0x00);
          SendByte(VDM|RWB_READ|(s*0x20+0x18));
          for(;i < rx_size;i ++)
                buf[i] = SendByte(0x00);
     }
     NET_CS_SET;
     offset1 += rx_size;//����ʵ��������ַ,���´ζ�ȡ���յ������ݵ���ʼ��ַ
     WriteSocket2Byte(s, Sn_RX_RD, offset1);
     WriteSocket1Byte(s, Sn_CR, RECV);          //������ձ�־λ
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
     SendByte(VDM|RWB_READ|COMMON_R);//ͨ��SPI2д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���
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
     SendByte(VDM|RWB_READ|COMMON_R);//ͨ��SPI2д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���
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
     SendByte(VDM|RWB_READ|COMMON_R);//ͨ��SPI2д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���
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