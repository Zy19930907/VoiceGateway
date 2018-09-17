#include "CPU.h"
#include "sys.h"
#include "W5500.h"
#include "Uart.h"
#include "FmInit.h"
#include "SD2068.h"

vu8 Timer125MsFlag;
extern vu16 SYS_TICK,SYS_TickSec;
extern CVoice Voice;

void ClockInit(void)
{
     Stm32_Clock_Init(9);
}

void LedIoInit(void)
{
// PC9-----RUN
	RCC->APB2ENR |= 0x10;   //PC 口
	GPIOC ->CRH  &= 0xFFFFFF0F;
	GPIOC ->CRH  |= 0x00000030;
}

void CpuInit(void)
{
	ClockInit();
	LedIoInit();
	FmInit();
	SD2068Init();
	Timer3Init(1000,71);     // 1MS 定时器
	Timer2Init(40000,8);     // 5MS 定时器
	Timer4Init(10000,7199);   // 1S 定时器
	Dma1Init();              // RS485接收数据
	Dma2Init();              // RS485发送数据
	UartInit(72,RS485_BAUD);
}

void Timer3Init(unsigned int arr,unsigned int psc)
{
	RCC->APB1ENR|=1<<1;//TIM3时钟使能    
 	TIM3->ARR=arr;  //设定计数器自动重装值//刚好1ms    
	TIM3->PSC=psc;  //预分频器7200,得到10Khz的计数时钟

	TIM3->DIER|= 1<<0;   //允许更新中断				
	TIM3->DIER|= 1<<6;   //允许触发中断	   
	TIM3->CR1 |= 0x00000001;
  	MY_NVIC_Init(1,3,TIM3_IRQChannel,2);
}


void Timer2Init(unsigned int arr,unsigned int psc)
{
    RCC->APB1ENR |= 0x01; //TIM2时钟使能    
 	TIM2->ARR = arr;   
	TIM2->PSC = psc;

	TIM2->DIER|= 1<<0;   //允许更新中断				
	TIM2->DIER|= 1<<6;   //允许触发中断	   

  	MY_NVIC_Init(1,0,TIM2_IRQChannel,2);//抢占1，子优先级3，组2	
}

void Timer4Init(unsigned int arr,unsigned int psc)
{
	RCC->APB1ENR |= 0x04; //TIM4时钟使能    
 	TIM4->ARR = arr;   
	TIM4->PSC = psc;

	TIM4->DIER|= 1<<0;   //允许更新中断				
	TIM4->DIER|= 1<<6;   //允许触发中断	
	TIM4->CR1 |= 0x00000001;     
  	MY_NVIC_Init(1,2,TIM4_IRQChannel,2);//抢占1，子优先级3，组2	
}

//初始化独立看门狗
//prer:分频数:0~7(只有低3位有效!)
//分频因子=4*2^prer.但最大值只能是256!
//rlr:重装载寄存器值:低11位有效.
//时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
void IWDG_Init(u8 prer,u16 rlr) 
{
	IWDG->KR = 0X5555;//使能对IWDG->PR和IWDG->RLR的写		 										  
  	IWDG->PR = prer;  //设置分频系数   
  	IWDG->RLR = rlr;  //从加载寄存器 IWDG->RLR  
	IWDG->KR = 0XAAAA;//reload											   
  	IWDG->KR = 0XCCCC;//使能看门狗	
}
//喂独立看门狗
void IWDG_Feed(void)
{
	IWDG->KR = 0XAAAA;//reload											   
}


void TIM3_IRQHandler(void)
{ 	
	if(TIM3->SR&0x0001)//溢出中断
		SYS_TICK ++;	    	
	TIM3->SR&=~(1<<0);//清除中断标志位 		   
}

void TIM4_IRQHandler(void)
{
	if(TIM4->SR&0x0001)//溢出中断
		SYS_TickSec ++;
	TIM4->SR&=~(1<<0);//清除中断标志位 		   
}

void TIM2_IRQHandler(void)
{ 	
	if(TIM2->SR&0x0001)//溢出中断
	{
		if(Voice.Status == Send)
			Voice.MsCnt++;
		else
			Voice.MsCnt = 0;
		if(Voice.MsCnt >= 25)
		{	
			Voice.MsCnt = 0;
			Timer125MsFlag = 1;
		}	  
	}		
	TIM2->SR&=~(1<<0);//清除中断标志位 		   
}
