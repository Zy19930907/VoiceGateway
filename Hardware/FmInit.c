#include "sys.h"
#include "FmInit.h"
#include "public.h"



// WP---PC0   SCL----PC1   SDA----PC2
// WP---PC3   SCL----PA0   SDA----PA1
// 先初始化FM1

void FmInit(void)
{  /*
     RCC->APB2ENR |= 0x10;   //PC 口
     GPIOC ->CRL  &= 0xFFFFF000;
     GPIOC ->CRL  |= 0x00000333;
     delay_us(2);
     FramWriteEnable;    */

     RCC->APB2ENR |= 0x10;   //PC 口
     RCC->APB2ENR |= 0x04; 
     GPIOC ->CRL  &= 0xFFFF0FFF;
     GPIOC ->CRL  |= 0x00003000;
     GPIOA ->CRL  &= 0xFFFFFF00;
     GPIOA ->CRL  |= 0x00000033;
     delay_us(2);
}

void FramStart(void)
{
     FM_SDA_OUT;delay_us(2);
     FM_SDA_HIGH;delay_us(2);
     FM_SCL_HIGH;
     delay_us(4);
     FM_SDA_LOW;
     delay_us(4);
     FM_SCL_LOW;
}
void FramStop(void)
{
     FM_SDA_OUT;delay_us(2);
     FM_SCL_LOW;delay_us(2);
     FM_SDA_LOW;
     delay_us(4);
     FM_SCL_HIGH;delay_us(2);
     FM_SDA_HIGH; 
     delay_us(4);	
}

void FramAck(void)
{
     FM_SCL_LOW;delay_us(2);
     FM_SDA_OUT;delay_us(2);
     FM_SDA_LOW;
     delay_us(2);
     FM_SCL_HIGH;
     delay_us(2);
     FM_SCL_LOW;
     delay_us(4);
}
	    
void FramNAck(void)
{
     FM_SCL_LOW;delay_us(20);
     FM_SDA_OUT;delay_us(20);
     FM_SDA_HIGH;
     delay_us(20);
     FM_SCL_HIGH;
     delay_us(20);
     FM_SCL_LOW;
}					 				     
		  
void FramSendByte(u8 txd)
{                        
     u8 t;   
     FM_SDA_OUT; delay_us(2);   
     FM_SCL_LOW;
     for(t=0;t<8;t++)
     {              
          if((txd&0x80)>>7)
               FM_SDA_HIGH;
          else
               FM_SDA_LOW;
          txd<<=1; 	  
          delay_us(2);
          FM_SCL_HIGH;
          delay_us(2); 
          FM_SCL_LOW;	
          delay_us(2);
     }	 
}
u8 FramReadByte(void)
{
     u8 i,receive=0;
     FM_SDA_IN;delay_us(2);
     for(i=0;i<8;i++ )
     {
          FM_SCL_LOW; 
          delay_us(2);
          FM_SCL_HIGH;
          receive<<=1;
          if(FM_SDA_VAL)
               receive++;   
          delay_us(1); 
     }
     FM_SCL_LOW;
     return receive;
}

void FramWriteBurstByte(u16 Addr, u8 *Buf, u16 len)
{
     u16 i;
     FramWriteEnable;
     FramStart();  
     FramSendByte(0xA0); 
     FramAck();
     FramSendByte(Addr >> 8); 
     FramAck(); 	 
     FramSendByte(Addr&0xFF); 
     FramAck();
     for(i = 0;i < len;i++)
     {
          FramSendByte(Buf[i]); 						   
          FramAck();
     }
     FramStop();
     FramWriteDisable; 
}

void FramReadBurstByte(u16 Addr, u8 *Buf, u16 len)
{
     u16 i;
     FramWriteEnable;
     FramStart();  
     FramSendByte(0xA0);	   
     FramAck(); 
     FramSendByte(Addr >> 8);
     FramAck();
     FramSendByte(Addr&0xFF);
     FramAck();	    
     FramStart();  	 	   
     FramSendByte(0xA1);		   
     FramAck();
     for(i = 0;i < len; i ++)
     {
          Buf[i] = FramReadByte();
          if(i == (len-1))
               FramNAck();
          else
               FramAck();
     }
     FramStop();
     FramWriteDisable; 
}

