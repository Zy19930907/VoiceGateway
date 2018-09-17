#include "Can.h"
#include "Sys.h"
#include "CanPro.h"

extern CCan Can;
extern vu16 SYS_TICK;
CanRxMsg RxMessage;

u32 CanBaudValue(u8 Pclk,u16 Baud)
{
	u32 BtrValue = 0;
	BtrValue += 0x00;
	BtrValue <<= 4;//重同步跳跃宽度1tq
	BtrValue += 0x07;
	BtrValue <<= 4;//时间段2 8tq
	BtrValue += 0x07;
	BtrValue <<= 16;//时间段1 8tq
	BtrValue += (((((u32)Pclk*1000000)/17/Baud) -1) & 0x000003FF);
	return BtrValue;
}

void CAN_IO_Init(void)
{
	RCC->APB2ENR |= (1<<0);//使能复用功能时钟
	RCC->APB2ENR |= (1<<2);//使能GPIOA时钟
	GPIOA->CRH &= 0xFFF00FFF;
	GPIOA->CRH |= 0x000B8000;
}

void CanRegisterConfig(void)
{     
	u16 tick;
	u8 i;
	
	RCC->APB1RSTR |= 1 << 25;//CAN时钟复位
	RCC->APB1RSTR &= ~(1 << 25);
	RCC->APB1ENR  |= 1 << 25;//CAN时钟使能
	//CAN->MCR |= (1 << 15);//CAN复位
     CAN->MCR |= (1 << 0); //申请进入CAN配置模式
	while((CAN->MSR & 0x00000001) != 0x00000001)
	{
		if(i++>200)
			return;
	}//等待进入CAN配置模式
	i=0;
	CAN->MCR &= ~(1<<1);//退出睡眠模式
	
	CAN->BTR = CanBaudValue(36,5000);//波特率5K
	tick = SYS_TICK;
	while(MsTickDiff(tick) < 5);
     
	CAN->FMR |= 1<<0;//过滤器配置模式
	
	CAN->FS1R |= 0x00000003;//过滤器组1、2位宽32位
	CAN->FM1R &= ~0x00000003;//过滤器组1、2屏蔽位模式
	CAN->FFA1R &= ~0x00000001;
	CAN->FFA1R |=0x00000002;//过滤器组1关联FIFO0，过滤器组2关联FIFO1
		
	CAN->sFilterRegister[0].FR1 = 0x00000000;
	CAN->sFilterRegister[0].FR2 = 0x00000000;//屏蔽器只关注地址位
	
	CAN->sFilterRegister[1].FR1 = 0x00000000;
	CAN->sFilterRegister[1].FR2 = 0x00000000;//屏蔽器只关注D15 控制标志位
	CAN->FA1R |= 0x00000003;//激活过滤器1、2
	CAN->FMR &= 0<<0;
	CAN->MCR |= (1<<6);//离线模式自动恢复
	CAN->MCR &= ~(1 << 0); //申请进入CAN正常模式
	while((CAN->MSR & 0x00000001) != 0x00000000)
	{
		if(i++>200)
			return;
	}//等待进入CAN正常模式

     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);	
	//CAN_ITConfig(CAN_IT_FMP0, ENABLE); 
	//MY_NVIC_Init(3,3,USB_LP_CAN_RX0_IRQChannel,2);//组2，最低优先级
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	//CAN_Receive(CAN_FIFO0, &RxMessage);
	Can.Status = RXOVER;		
}


u32 CanIdSwitch(u32 CanId, u8 SwitchDir)
{
	u32 temp = 0;
	switch (SwitchDir)
	{
	case CANIDSWITCH_B2F:
		CanId >>= 3;
		temp = (CanId >> 18)&0x000003FF;
		temp |= (CanId << 11)&0x1FFFF800;
		break;
	case CANIDSWITCH_F2B:
		temp = ((CanId & 0x000007FF) << 18);
		temp |= ((CanId & 0x7FFFF800) >> 11);
		temp <<= 3; 
		temp|= 0x00000004;
		break;
	}
	return temp;
}

void Can1RecvFunc(void)
{
	u32 t;
	if(CAN->RF0R & 0x00000003)
     {
		FIFO0RecvDeal:
		Can.Len = (CAN->sFIFOMailBox[0].RDTR & 0x0000000F);
		 if(Can.Len >= 8)
			Can.Len = 8;
		t= (u32)0x1FFFFFFF & (CAN->sFIFOMailBox[0].RIR >> 3);
		Can.ID = (t >> 18) + ((t&0x3FFFF)<<11);
		Can.Buf[0]=CAN->sFIFOMailBox[0].RDLR&0xFF;
		Can.Buf[1]=(CAN->sFIFOMailBox[0].RDLR>>8)&0xFF;
		Can.Buf[2]=(CAN->sFIFOMailBox[0].RDLR>>16)&0xFF;
		Can.Buf[3]=(CAN->sFIFOMailBox[0].RDLR>>24)&0xFF;    
		Can.Buf[4]= CAN->sFIFOMailBox[0].RDHR&0xFF;
		Can.Buf[5]=(CAN->sFIFOMailBox[0].RDHR>>8)&0xFF;
		Can.Buf[6]=(CAN->sFIFOMailBox[0].RDHR>>16)&0xFF;
		Can.Buf[7]=(CAN->sFIFOMailBox[0].RDHR>>24)&0xFF;		 
		HandleCanData(Can.ID,1);		
        CAN->RF0R |= (1 << 5);//释放FIFO0
		while(CAN->RF0R  & 0x00000020);
		if(CAN->RF0R & 0x00000003)//FIFO0中还存在未读报文
			goto FIFO0RecvDeal;
		if(CAN->RF0R & 0x00000010)
			CAN->RF0R &= ~(1 << 4);
		if(CAN->RF0R & 0x00000008)
			CAN->RF0R &= ~(1 << 3);
     }
     if(CAN->RF1R & 0x00000003)
     {
		FIFO1RecvDeal:
		Can.Len = (CAN->sFIFOMailBox[1].RDTR & 0x0000000F);
		 if(Can.Len >= 8)
			Can.Len = 8;
		t = (u32)0x1FFFFFFF & (CAN->sFIFOMailBox[1].RIR >> 3);
		Can.ID = (t >> 18) + ((t&0x3FFFF)<<11);
		Can.Buf[0]=CAN->sFIFOMailBox[1].RDLR&0xFF;
		Can.Buf[1]=(CAN->sFIFOMailBox[1].RDLR>>8)&0xFF;
		Can.Buf[2]=(CAN->sFIFOMailBox[1].RDLR>>16)&0xFF;
		Can.Buf[3]=(CAN->sFIFOMailBox[1].RDLR>>24)&0xFF;    
		Can.Buf[4]=CAN->sFIFOMailBox[1].RDHR&0xFF;
		Can.Buf[5]=(CAN->sFIFOMailBox[1].RDHR>>8)&0xFF;
		Can.Buf[6]=(CAN->sFIFOMailBox[1].RDHR>>16)&0xFF;
		Can.Buf[7]=(CAN->sFIFOMailBox[1].RDHR>>24)&0xFF; 
		HandleCanData(Can.ID,1);
        CAN->RF1R |= (1 << 5);//释放FIFO1
		while(CAN->RF1R & 0x00000020);
		if(CAN->RF1R & 0x00000003)//FIFO1中还存在未读报文
			goto FIFO1RecvDeal;
		if(CAN->RF1R & 0x00000010)
			CAN->RF1R &= ~(1 << 4);
		if(CAN->RF1R & 0x00000008)
			CAN->RF1R &= ~(1 << 3);
     }
}


void Can1Init(void)
{
	CAN_IO_Init();
	CanRegisterConfig();
}

/*
 * 第二路CAN
 * PB11---CS  PC10---MISO   PC11---MOSI   PC12---SCLK   
 * PD2---INT 
 */
void Can2Init(void)
{
     RCC->APB2ENR |= 0x38;
     GPIOC ->CRH  &= 0xFFF000FF;
     GPIOC ->CRH  |= 0x00033800;
     
     GPIOB ->CRH  &= 0xFFFF0FFF;
     GPIOB ->CRH  |= 0x00003000;
     
     GPIOD ->CRL  &= 0xFFFFF0FF;
     GPIOD ->CRL  |= 0x00000800;
     Mcp2515Init(2);
}
/*
 * 第三路CAN
 * PB5---CS  PB6---MISO   PB7---MOSI   PB8---SCLK   
 * PB9---INT 
 */
void Can3Init(void)
{
     RCC->APB2ENR |= 0x08;
     GPIOB ->CRL  &= 0x000FFFFF;
     GPIOB ->CRL  |= 0x38300000;
     
     GPIOB ->CRH  &= 0xFFFFFF00;
     GPIOB ->CRH  |= 0x00000083;
     Mcp2515Init(3);
}


void CLR_SCLK(u8 t)
{
     switch(t)
     {
     case 2: CLR_SCLK_2;break;
     case 3: CLR_SCLK_3;break;
     }
}
void SET_SCLK(u8 t)
{
     switch(t)
     {
     case 2: SET_SCLK_2;break;
     case 3: SET_SCLK_3;break;
     }
}
void CLR_MOSI(u8 t)
{
     switch(t)
     {
     case 2: CLR_MOSI_2;break;
     case 3: CLR_MOSI_3;break;
     }
}

void SET_MOSI(u8 t)
{
     switch(t)
     {
     case 2: SET_MOSI_2;break;
     case 3: SET_MOSI_3;break;
     }
}
void CLR_CS(u8 t)
{
     switch(t)
     {
     case 2: CLR_CS_2;break;
     case 3: CLR_CS_3;break;
     }
}
void SET_CS(u8 t)
{
     switch(t)
     {
     case 2: SET_CS_2;break;
     case 3: SET_CS_3;break;
     }
}

u8 GetMISO(u8 t)
{
     u8 dat;
     switch(t)
     {
		case 2: dat = MISO_2;break;
		case 3: dat = MISO_3;break;
     }
     return dat;
}

u8  SendByteCan(u8 index,u8 val)
{
     unsigned char i, dat;
     dat = 0;
     CLR_SCLK(index);
     for(i = 0; i < 8; i ++)
     {
          if(val & 0x80)
               SET_MOSI(index);
          else
               CLR_MOSI(index);
          val <<= 1;
          SET_SCLK(index);
          dat <<= 1;
          if(GetMISO(index))
               dat ++;     
          CLR_SCLK(index);
     }
	return dat;
}

void MCP2515Reset(u8 index)
{
     CLR_CS(index);
     SendByteCan(index,CAN_RESET);
     SET_CS(index);
}

void WriteRegCan(u8 index,u8 addr, u8 value)
{
     CLR_CS(index);
     SendByteCan(index,CAN_WRITE);
     SendByteCan(index,addr);
     SendByteCan(index,value);
     SET_CS(index);
}

void WriteBurstRegCan(u8 index,u8 addr, u8 *buf, u8 len)
{
     u8 i;
     CLR_CS(index);
     SendByteCan(index,CAN_WRITE);
     SendByteCan(index,addr);
     for(i = 0;i < len;i ++)
         SendByteCan(index,buf[i]);
     SET_CS(index);
}
u8 ReadRegCan(u8 index,u8 addr)
{
     u8 value;
     CLR_CS(index);
     SendByteCan(index,CAN_READ);
     SendByteCan(index,addr);
     value = SendByteCan(index,0x00); 
     SET_CS(index);
     return value;
}

void ReadBurstRegCan(u8 index,u8 addr,u8 *buf,u8 len)
{
     u8 i;
     CLR_CS(index);
     SendByteCan(index,CAN_READ);
     SendByteCan(index,addr);
     for(i = 0;i < len;i ++)
         buf[i] = SendByteCan(index,0x00);
     SET_CS(index);
}


void ModifyReg(u8 index,u8 addr,u8 mask,u8 val)
{
     CLR_CS(index);
     SendByteCan(index,CAN_BIT_MODIFY);
     SendByteCan(index,addr);
     SendByteCan(index,mask);
     SendByteCan(index,val);
     SET_CS(index);
}

u8 ReadStatus(u8 index)
{
     u8 temp;
     CLR_CS(index);
     SendByteCan(index,CAN_RD_STATUS);
     temp = SendByteCan(index,0x00);
     SET_CS(index);
     return temp;
}

void Mcp2515Init(u8 index)
{
     u16 tick;
     MCP2515Reset(index);
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);
   
     WriteRegCan(index,CANCTRL,CONFIG_MODE);
     
     WriteRegCan(index,CNF1,0x31);
     WriteRegCan(index,CNF2,0xA4);
     WriteRegCan(index,CNF3, 0x04);
    
     WriteRegCan(index,CANINTE,0x00);
     WriteRegCan(index,CANINTF,0x00);
     WriteRegCan(index,RXB0CTRL,0x40);
     WriteRegCan(index,RXB1CTRL,0x40);

     WriteRegCan(index,RXM0EIDH,0x00);  
     WriteRegCan(index,RXM0EIDL,0x08);  
     WriteRegCan(index,RXM0SIDH,0x00);  
     WriteRegCan(index,RXM0SIDL,0x00);  
     
     WriteRegCan(index,RXF0EIDH,0x00);  
     WriteRegCan(index,RXF0EIDL,0x08);  
     WriteRegCan(index,RXF0SIDH,0x00);  
     WriteRegCan(index,RXF0SIDL,0x08);  
     
     WriteRegCan(index,TXRTSCTRL,0);
     WriteRegCan(index,BFPCTRL,0);
     WriteRegCan(index,CANCTRL,NORMAL_MODE);
}
