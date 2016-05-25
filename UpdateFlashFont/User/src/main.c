/**
编译器版本：MDK3.5  基于STM32 STD3.0库
测试DEMO表述： SD卡上的Tini-FatFs0.07，测试了将SD卡上的txt类型文件内容通过串口输出,
               测试读文件速度大约在1M BYTEs/s。
			    
硬件平台：HY_STM32_100P开发板
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "integer.h"
#include "usart.h"
#include "data_parse.h"
#include "jpg_create.h"
#include "piclib.h"
#include "delay.h"
#include "timer.h"
#include "libtimer.h"
#include "sdcard.h"
#include "lcd.h"
#include "sim900a.h"
#include "keys.h"
#include "cJSON.h"
#include "cjsonutil.h"
#include "ff.h"
#include "diskio.h"
#include "fontupd.h"
#include "text.h"
#include "app_task.h"

extern TimerHandleT g_tHttpGetQRCode;
extern TimerHandleT g_tHttpGetVersion;
extern u32 g_uVersion;
u32 g_uGetQRCodeQuickTime = 5000;

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

//先用开发板测试是否可行
int main(void)
{ 
		u8 key = 0;
	
		SystemInit();
				
		/* Setup SysTick Timer for 1 msec interrupts  */
		if (SysTick_Config(SystemFrequency / 1000))
		{ 
			/* Capture error */ 
			while (1);
		}
				
		LCD_Init();
		
		piclib_init();
		
		LCD_Clear(WHITE);	
	
		POINT_COLOR=RED;
		
		Show_Str(60,110,200,16,"System Init",16,0);	

		//先测试是否能正常接收数据
		if(SD_GetStatus() == SD_OK)
		{
			 Show_Str(60,130,200,16,"SD Init OK",16,0);
		}
		
		MountSD();
		
		while(font_init()) //检查字库
		{   
			LCD_Clear(WHITE);		   	//清屏
			POINT_COLOR=RED;			//设置字体为红色	   	   	  
			LCD_ShowString(60,50,200,16,16,"M3S STM32");
																
			LCD_ShowString(60,70,200,16,16,"SD Card OK");
			LCD_ShowString(60,90,200,16,16,"Font Updating...");
			key=update_font(20,110,16,0);//从SD卡更新
			while(key)//更新失败		
			{			 		  
				LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
				delay_nms(200);
				LCD_Fill(20,110,200+20,110+16,WHITE);
				delay_nms(200);		       
			} 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Success!");
			delay_nms(1500);	
			LCD_Clear(WHITE);//清屏	       
		}  
	
		POINT_COLOR=RED;      
		Show_Str(60,50,200,16,"字库更新完毕",16,0);				    	 	
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
