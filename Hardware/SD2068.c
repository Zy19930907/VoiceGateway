#include <stm32f10x_lib.h>
#include "SD2068.h"

/*
SCL       PA6
SDA       PA7
INT       PC4
*/

CTime Time;

void SD2068Init(void)
{
    RCC->APB2ENR |= 0x04;
   
    GPIOA->CRL &= 0X00FFFFFF;
    GPIOA->CRL |= 0X33000000;
}


void TIM_Start(void)
{
     TIM_SDA_OUT;//delay_us(4);
     TIM_SDA_HIGH;//delay_us(4);
     TIM_SCL_HIGH;
     delay_us(4);
     TIM_SDA_LOW;
     delay_us(4);
     TIM_SCL_LOW;
}
void TIM_Stop(void)
{
     TIM_SDA_OUT;//delay_us(4);
     TIM_SCL_LOW;//delay_us(4);
     TIM_SDA_LOW;//delay_us(4);
     delay_us(4);
     TIM_SCL_HIGH;//delay_us(4);
     TIM_SDA_HIGH;
     delay_us(4);
}
u8 TIM_Wait_ACK(void)
{
     u8 ucErr=0;
     TIM_SDA_IN;
     
    // delay_us(4);
     TIM_SCL_HIGH;
   //  delay_us(1);	 
     while(TIM_SDA_VAL)
     {
          ucErr++;
          if(ucErr>250)
          {
               TIM_Stop();
               return 1;
          }
     }
     TIM_SCL_LOW;	   
     return 0;  
}

void TIM_Ack(void)
{
     TIM_SCL_LOW;//delay_us(4);
     TIM_SDA_OUT;//delay_us(4);
     TIM_SDA_LOW;
     delay_us(2);
     TIM_SCL_HIGH;
     delay_us(2);
     TIM_SCL_LOW;
}
	    
void TIM_NAck(void)
{
     TIM_SCL_LOW;//delay_us(4);
     TIM_SDA_OUT;//delay_us(4);
     TIM_SDA_HIGH;
     delay_us(2);
     TIM_SCL_HIGH;
     delay_us(2);
     TIM_SCL_LOW;
}					 				     
		  
void TIM_Send_Byte(u8 txd)
{                        
     u8 t;   
     TIM_SDA_OUT;//delay_us(4);    
     TIM_SCL_LOW;//delay_us(4);
     for(t=0;t<8;t++)
     {              
          if(txd&0x80)
               TIM_SDA_HIGH;
          else
               TIM_SDA_LOW;
          txd<<=1; 	  
          delay_us(2);
          TIM_SCL_HIGH;
          delay_us(2); 
          TIM_SCL_LOW;	
          delay_us(2);
     }	 
} 	    

u8 TIM_Read_Byte(void)
{
     unsigned char i,receive=0;
     TIM_SDA_IN;//delay_us(4);
     for(i=0;i<8;i++ )
     {
          TIM_SCL_LOW; 
          delay_us(2);
          TIM_SCL_HIGH;
          receive<<=1;
          if(TIM_SDA_VAL)
          receive++;   
          delay_us(1); 
     }
     TIM_SCL_LOW;
     return receive;
}

void ReadTime(void)
{
     u8 i; 
     TIM_Start();
     TIM_Send_Byte(0x65);
     TIM_Wait_ACK();
     for(i = 7;i > 0;i --)
     {
          Time.Hex[i-1] = TIM_Read_Byte();
          if(i != 1)
               TIM_Ack(); 
     }
     TIM_NAck();
     TIM_Stop();
     Time.Hex[4] &= 0x7f;  //小时，把最高位过滤
    // 转换成BCD码
     for(i = 0;i < 7;i ++)
         Time.Buf[i] = (Time.Hex[i]/16)*10+Time.Hex[i]%16;
     Time.AllSecond = Time.Buf[5] * 60 + Time.Buf[6];
}

/**********************************************************************
 * 允许写入
***********************************************************************/  
void WriteOn(void)
{		
     TIM_Start();
     TIM_Send_Byte(0x64);      
     TIM_Wait_ACK();   
     TIM_Send_Byte(0x10);   
     TIM_Wait_ACK();	
     TIM_Send_Byte(0x80);   
     TIM_Wait_ACK();
     TIM_Stop(); 
	
     TIM_Start();
     TIM_Send_Byte(0x64);      
     TIM_Wait_ACK();   
     TIM_Send_Byte(0x0F);  
     TIM_Wait_ACK();	
     TIM_Send_Byte(0xFF);    
     TIM_Wait_ACK();
     TIM_Stop(); 
}

/**********************************************************************
 * 禁止写入
***********************************************************************/  
void WriteOff(void)
{
     TIM_Start();
     TIM_Send_Byte(0x64);      
     TIM_Wait_ACK();   
     TIM_Send_Byte(0x0F);  
     TIM_Wait_ACK();	
     TIM_Send_Byte(0x7B);    
     TIM_Wait_ACK();
     TIM_Send_Byte(0x0);    
     TIM_Wait_ACK();
     TIM_Stop(); 
}

void WriteTime(void)
{
     WriteOn();				//允许写入
     TIM_Start();
     TIM_Send_Byte(0x64);      
     TIM_Wait_ACK();   
     TIM_Send_Byte(0x00);			//设置起始地址
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[6]);		//秒  
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[5]);		//分      
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[4]|0x80);    //时  24h制    
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[3]);		//date
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[2]);		//月     
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[1]);		//日      
     TIM_Wait_ACK();	
     TIM_Send_Byte(Time.Buf[0]);		//年      
     TIM_Wait_ACK();	
     TIM_Stop();
     WriteOff();             // 关闭写
}

