#include "sys.h"
#include "Uart.h"
#include "W5500.h"

extern CVoice Voice;
extern vu16 SYS_TICK;
extern CRs485 Rs485;

void UartInit(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp = (float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa = temp;				 //得到整数部分
	fraction = (temp-mantissa)*16; //得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
	RCC->APB2ENR |= 1<<2;   //使能PORTA口时钟  
	RCC->APB2ENR |= 1<<14;  //使能串口时钟 
	GPIOA->CRH &= 0xFFFFF000; 
	GPIOA->CRH |= 0x000008B3;//IO状态设置
		  
	RCC->APB2RSTR |= 1<<14;   //复位串口1
	RCC->APB2RSTR &= ~(1<<14);//停止复位	   	   
	//波特率设置
 	USART1->BRR = mantissa; // 波特率设置
     	
    USART1->CR3 |= 0xC0; 
	USART1->CR1 |= 0X200C;  //1位停止,无校验位.
     
	//使能接收中断
	USART1->CR1|=1<<4;    //idle中断	    	
	MY_NVIC_Init(1,1,USART1_IRQChannel,2);//组2，最低优先级 

    SetRs485Rec;
}

// RS485 接收数据，通过DMA接收并且传输至PC
// 通道5--串口接收数据
void Dma1Init(void)
{
     u16 tick;
     RCC ->AHBENR |= 0x01;
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);

     DMA1_Channel5 ->CPAR = (u32)&(USART1->DR);         //外设地址
     DMA1_Channel5 ->CMAR = (u32)&Rs485.Buf[0];         //存储器地址
     DMA1_Channel5 ->CNDTR = MaxDmaLen;
     DMA1_Channel5 ->CCR = 0x00000000;           //复位
     DMA1_Channel5 ->CCR |= 0<<4;             //传输方向，从外设读数据到存储器
     DMA1_Channel5 ->CCR |= 1<<5;               //执行循环操作
     DMA1_Channel5 ->CCR |= 0<<6;             //外设地址不递增
     DMA1_Channel5 ->CCR |= 1<<7;               //存储器地址递增
     DMA1_Channel5 ->CCR |= 0<<8;               //外设数据宽度8BIT
     DMA1_Channel5 ->CCR |= 0<<10;              //存储数据宽度8bit
     DMA1_Channel5 ->CCR |= 1<<12;              //中等优先级
     DMA1_Channel5 ->CCR |= 0<<14;            //非存储器到存储器 
     MY_NVIC_Init(1,2,DMA1_Channel5_IRQChannel,1); 
     DMA1_Channel5 ->CCR |= 0x07;          //传输过半中断、传输完成中断、通道使能 
}

// 网络收到数据，通过DMA向RS485发送数据到广播
// 通道4--串口发送数据，
void Dma2Init(void)
{
     u16 tick;
     RCC ->AHBENR |= 0x01;
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);

     DMA1_Channel4 ->CPAR = (u32)&(USART1->DR);  //外设地址
     DMA1_Channel4 ->CMAR = (u32)&Rs485.Buf[0];//存储器地址
     DMA1_Channel4 ->CCR = 0x00000000;         //复位
     DMA1_Channel4 ->CCR |= 1<<4;             //传输方向，从存储器读数据输出到外设
     DMA1_Channel4 ->CCR |= 0<<5;             //不执行循环操作
     DMA1_Channel4 ->CCR |= 0<<6;             //外设地址不递增
     DMA1_Channel4 ->CCR |= 1<<7;             //存储器地址递增
     DMA1_Channel4 ->CCR |= 0<<8;            //外设数据宽度8BIT
     DMA1_Channel4 ->CCR |= 0<<10;           //存储数据宽度8bit
     DMA1_Channel4 ->CCR |= 1<<12;           //中等优先级
     DMA1_Channel4 ->CCR |= 0<<14;            //非存储器到存储器 
}

// 往PC传输,将串口收到的数据搬移到缓冲区中
void DMA1_Channel5_IRQHandler(void)
{
	u16 i,cnt;
	if((DMA1 ->ISR) & 0x00020000)     //缓冲区全满
		i = 505;
	else                              //缓冲区半满
		i = 0;
	DMA1 -> IFCR = 0x00070000;
	cnt = i + 500; 
	if(Rs485.Buf[i+4]!=0x00)
	{
		Voice.DestAddr = Rs485.Buf[i+4];  //前面4个字节是帧头 0x55 AA 55 AA
		for(;i < cnt;i ++)
		Voice.RecBuf[Voice.W++] = Rs485.Buf[i+5];
		if(Voice.W == MaxVoiceLen)
		Voice.W = 0;   
		Voice.Dir = DeviceToPc;
	}
}

// DMA已经完成串口数据发送
void DMA1_Channel4_IRQHandler(void)
{
     DMA1 ->IFCR = 0x00007000;
     DMA1_Channel4 ->CCR &= ~0x01;   
}

void USART1_IRQHandler(void)
{
	u16 temp;
	if(USART1 -> SR & 0x10)    // 空闲中断 , 先关闭DMA通道，清零计数，在打开
	{
		temp = MaxDmaLen - DMA1_Channel5 ->CNDTR;
		if(temp == 10)         // 广播终端回复信息，理论上不会超过50个
		{
			Rs485.Status = RS485Receive;
			Rs485.Cnt = 10;
		}
		DMA1_Channel5 ->CCR &= ~0x01;   
		DMA1_Channel5 ->CNDTR = MaxDmaLen;
		DMA1_Channel5 ->CCR |=  0x01; 
		temp |= USART1->DR;
	}
}
