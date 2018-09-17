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
	RCC->APB2ENR |= 0x10;   //PC ��
	GPIOC ->CRH  &= 0xFFFFFF0F;
	GPIOC ->CRH  |= 0x00000030;
}

void CpuInit(void)
{
	ClockInit();
	LedIoInit();
	FmInit();
	SD2068Init();
	Timer3Init(1000,71);     // 1MS ��ʱ��
	Timer2Init(40000,8);     // 5MS ��ʱ��
	Timer4Init(10000,7199);   // 1S ��ʱ��
	Dma1Init();              // RS485��������
	Dma2Init();              // RS485��������
	UartInit(72,RS485_BAUD);
}

void Timer3Init(unsigned int arr,unsigned int psc)
{
	RCC->APB1ENR|=1<<1;//TIM3ʱ��ʹ��    
 	TIM3->ARR=arr;  //�趨�������Զ���װֵ//�պ�1ms    
	TIM3->PSC=psc;  //Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��

	TIM3->DIER|= 1<<0;   //��������ж�				
	TIM3->DIER|= 1<<6;   //�������ж�	   
	TIM3->CR1 |= 0x00000001;
  	MY_NVIC_Init(1,3,TIM3_IRQChannel,2);
}


void Timer2Init(unsigned int arr,unsigned int psc)
{
    RCC->APB1ENR |= 0x01; //TIM2ʱ��ʹ��    
 	TIM2->ARR = arr;   
	TIM2->PSC = psc;

	TIM2->DIER|= 1<<0;   //��������ж�				
	TIM2->DIER|= 1<<6;   //�������ж�	   

  	MY_NVIC_Init(1,0,TIM2_IRQChannel,2);//��ռ1�������ȼ�3����2	
}

void Timer4Init(unsigned int arr,unsigned int psc)
{
	RCC->APB1ENR |= 0x04; //TIM4ʱ��ʹ��    
 	TIM4->ARR = arr;   
	TIM4->PSC = psc;

	TIM4->DIER|= 1<<0;   //��������ж�				
	TIM4->DIER|= 1<<6;   //�������ж�	
	TIM4->CR1 |= 0x00000001;     
  	MY_NVIC_Init(1,2,TIM4_IRQChannel,2);//��ռ1�������ȼ�3����2	
}

//��ʼ���������Ź�
//prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
//��Ƶ����=4*2^prer.�����ֵֻ����256!
//rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
//ʱ�����(���):Tout=((4*2^prer)*rlr)/40 (ms).
void IWDG_Init(u8 prer,u16 rlr) 
{
	IWDG->KR = 0X5555;//ʹ�ܶ�IWDG->PR��IWDG->RLR��д		 										  
  	IWDG->PR = prer;  //���÷�Ƶϵ��   
  	IWDG->RLR = rlr;  //�Ӽ��ؼĴ��� IWDG->RLR  
	IWDG->KR = 0XAAAA;//reload											   
  	IWDG->KR = 0XCCCC;//ʹ�ܿ��Ź�	
}
//ι�������Ź�
void IWDG_Feed(void)
{
	IWDG->KR = 0XAAAA;//reload											   
}


void TIM3_IRQHandler(void)
{ 	
	if(TIM3->SR&0x0001)//����ж�
		SYS_TICK ++;	    	
	TIM3->SR&=~(1<<0);//����жϱ�־λ 		   
}

void TIM4_IRQHandler(void)
{
	if(TIM4->SR&0x0001)//����ж�
		SYS_TickSec ++;
	TIM4->SR&=~(1<<0);//����жϱ�־λ 		   
}

void TIM2_IRQHandler(void)
{ 	
	if(TIM2->SR&0x0001)//����ж�
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
	TIM2->SR&=~(1<<0);//����жϱ�־λ 		   
}
