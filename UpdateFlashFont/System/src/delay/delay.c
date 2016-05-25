#include "delay.h"

void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}

//72MHZ
void delay_nus(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=10;  
      while(i--) ;    
   }
}

//72MHZ
void delay_nms(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=12000;  
      while(i--) ;    
   }
}
