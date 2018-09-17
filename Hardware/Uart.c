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
	temp = (float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa = temp;				 //�õ���������
	fraction = (temp-mantissa)*16; //�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
	RCC->APB2ENR |= 1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB2ENR |= 1<<14;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH &= 0xFFFFF000; 
	GPIOA->CRH |= 0x000008B3;//IO״̬����
		  
	RCC->APB2RSTR |= 1<<14;   //��λ����1
	RCC->APB2RSTR &= ~(1<<14);//ֹͣ��λ	   	   
	//����������
 	USART1->BRR = mantissa; // ����������
     	
    USART1->CR3 |= 0xC0; 
	USART1->CR1 |= 0X200C;  //1λֹͣ,��У��λ.
     
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<4;    //idle�ж�	    	
	MY_NVIC_Init(1,1,USART1_IRQChannel,2);//��2��������ȼ� 

    SetRs485Rec;
}

// RS485 �������ݣ�ͨ��DMA���ղ��Ҵ�����PC
// ͨ��5--���ڽ�������
void Dma1Init(void)
{
     u16 tick;
     RCC ->AHBENR |= 0x01;
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);

     DMA1_Channel5 ->CPAR = (u32)&(USART1->DR);         //�����ַ
     DMA1_Channel5 ->CMAR = (u32)&Rs485.Buf[0];         //�洢����ַ
     DMA1_Channel5 ->CNDTR = MaxDmaLen;
     DMA1_Channel5 ->CCR = 0x00000000;           //��λ
     DMA1_Channel5 ->CCR |= 0<<4;             //���䷽�򣬴���������ݵ��洢��
     DMA1_Channel5 ->CCR |= 1<<5;               //ִ��ѭ������
     DMA1_Channel5 ->CCR |= 0<<6;             //�����ַ������
     DMA1_Channel5 ->CCR |= 1<<7;               //�洢����ַ����
     DMA1_Channel5 ->CCR |= 0<<8;               //�������ݿ��8BIT
     DMA1_Channel5 ->CCR |= 0<<10;              //�洢���ݿ��8bit
     DMA1_Channel5 ->CCR |= 1<<12;              //�е����ȼ�
     DMA1_Channel5 ->CCR |= 0<<14;            //�Ǵ洢�����洢�� 
     MY_NVIC_Init(1,2,DMA1_Channel5_IRQChannel,1); 
     DMA1_Channel5 ->CCR |= 0x07;          //��������жϡ���������жϡ�ͨ��ʹ�� 
}

// �����յ����ݣ�ͨ��DMA��RS485�������ݵ��㲥
// ͨ��4--���ڷ������ݣ�
void Dma2Init(void)
{
     u16 tick;
     RCC ->AHBENR |= 0x01;
     tick = SYS_TICK;
     while(MsTickDiff(tick) < 5);

     DMA1_Channel4 ->CPAR = (u32)&(USART1->DR);  //�����ַ
     DMA1_Channel4 ->CMAR = (u32)&Rs485.Buf[0];//�洢����ַ
     DMA1_Channel4 ->CCR = 0x00000000;         //��λ
     DMA1_Channel4 ->CCR |= 1<<4;             //���䷽�򣬴Ӵ洢�����������������
     DMA1_Channel4 ->CCR |= 0<<5;             //��ִ��ѭ������
     DMA1_Channel4 ->CCR |= 0<<6;             //�����ַ������
     DMA1_Channel4 ->CCR |= 1<<7;             //�洢����ַ����
     DMA1_Channel4 ->CCR |= 0<<8;            //�������ݿ��8BIT
     DMA1_Channel4 ->CCR |= 0<<10;           //�洢���ݿ��8bit
     DMA1_Channel4 ->CCR |= 1<<12;           //�е����ȼ�
     DMA1_Channel4 ->CCR |= 0<<14;            //�Ǵ洢�����洢�� 
}

// ��PC����,�������յ������ݰ��Ƶ���������
void DMA1_Channel5_IRQHandler(void)
{
	u16 i,cnt;
	if((DMA1 ->ISR) & 0x00020000)     //������ȫ��
		i = 505;
	else                              //����������
		i = 0;
	DMA1 -> IFCR = 0x00070000;
	cnt = i + 500; 
	if(Rs485.Buf[i+4]!=0x00)
	{
		Voice.DestAddr = Rs485.Buf[i+4];  //ǰ��4���ֽ���֡ͷ 0x55 AA 55 AA
		for(;i < cnt;i ++)
		Voice.RecBuf[Voice.W++] = Rs485.Buf[i+5];
		if(Voice.W == MaxVoiceLen)
		Voice.W = 0;   
		Voice.Dir = DeviceToPc;
	}
}

// DMA�Ѿ���ɴ������ݷ���
void DMA1_Channel4_IRQHandler(void)
{
     DMA1 ->IFCR = 0x00007000;
     DMA1_Channel4 ->CCR &= ~0x01;   
}

void USART1_IRQHandler(void)
{
	u16 temp;
	if(USART1 -> SR & 0x10)    // �����ж� , �ȹر�DMAͨ��������������ڴ�
	{
		temp = MaxDmaLen - DMA1_Channel5 ->CNDTR;
		if(temp == 10)         // �㲥�ն˻ظ���Ϣ�������ϲ��ᳬ��50��
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
