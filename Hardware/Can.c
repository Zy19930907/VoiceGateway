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
	BtrValue <<= 4;//��ͬ����Ծ���1tq
	BtrValue += 0x07;
	BtrValue <<= 4;//ʱ���2 8tq
	BtrValue += 0x07;
	BtrValue <<= 16;//ʱ���1 8tq
	BtrValue += (((((u32)Pclk*1000000)/17/Baud) -1) & 0x000003FF);
	return BtrValue;
}

void CAN_IO_Init(void)
{
	RCC->APB2ENR |= (1<<0);//ʹ�ܸ��ù���ʱ��
	RCC->APB2ENR |= (1<<2);//ʹ��GPIOAʱ��
	GPIOA->CRH &= 0xFFF00FFF;
	GPIOA->CRH |= 0x000B8000;
}

void CanRegisterConfig(void)
{     
	u16 tick;
	u8 i;
	
	RCC->APB1RSTR |= 1 << 25;//CANʱ�Ӹ�λ
	RCC->APB1RSTR &= ~(1 << 25);
	RCC->APB1ENR  |= 1 << 25;//CANʱ��ʹ��
	//CAN->MCR |= (1 << 15);//CAN��λ
     CAN->MCR |= (1 << 0); //�������CAN����ģʽ
	while((CAN->MSR & 0x00000001) != 0x00000001)
	{
		if(i++>200)
			return;
	}//�ȴ�����CAN����ģʽ
	i=0;
	CAN->MCR &= ~(1<<1);//�˳�˯��ģʽ
	
	CAN->BTR = CanBaudValue(36,5000);//������5K
	tick = SYS_TICK;
	while(MsTickDiff(tick) < 5);
     
	CAN->FMR |= 1<<0;//����������ģʽ
	
	CAN->FS1R |= 0x00000003;//��������1��2λ��32λ
	CAN->FM1R &= ~0x00000003;//��������1��2����λģʽ
	CAN->FFA1R &= ~0x00000001;
	CAN->FFA1R |=0x00000002;//��������1����FIFO0����������2����FIFO1
		
	CAN->sFilterRegister[0].FR1 = 0x00000000;
	CAN->sFilterRegister[0].FR2 = 0x00000000;//������ֻ��ע��ַλ
	
	CAN->sFilterRegister[1].FR1 = 0x00000000;
	CAN->sFilterRegister[1].FR2 = 0x00000000;//������ֻ��עD15 ���Ʊ�־λ
	CAN->FA1R |= 0x00000003;//���������1��2
	CAN->FMR &= 0<<0;
	CAN->MCR |= (1<<6);//����ģʽ�Զ��ָ�
	CAN->MCR &= ~(1 << 0); //�������CAN����ģʽ
	while((CAN->MSR & 0x00000001) != 0x00000000)
	{
		if(i++>200)
			return;
	}//�ȴ�����CAN����ģʽ

     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);	
	//CAN_ITConfig(CAN_IT_FMP0, ENABLE); 
	//MY_NVIC_Init(3,3,USB_LP_CAN_RX0_IRQChannel,2);//��2��������ȼ�
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
        CAN->RF0R |= (1 << 5);//�ͷ�FIFO0
		while(CAN->RF0R  & 0x00000020);
		if(CAN->RF0R & 0x00000003)//FIFO0�л�����δ������
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
        CAN->RF1R |= (1 << 5);//�ͷ�FIFO1
		while(CAN->RF1R & 0x00000020);
		if(CAN->RF1R & 0x00000003)//FIFO1�л�����δ������
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
 * �ڶ�·CAN
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
 * ����·CAN
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
