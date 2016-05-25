/**
�������汾��MDK3.5  ����STM32 STD3.0��
����DEMO������ SD���ϵ�Tini-FatFs0.07�������˽�SD���ϵ�txt�����ļ�����ͨ���������,
               ���Զ��ļ��ٶȴ�Լ��1M BYTEs/s��
			    
Ӳ��ƽ̨��HY_STM32_100P������
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

//���ÿ���������Ƿ����
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

		//�Ȳ����Ƿ���������������
		if(SD_GetStatus() == SD_OK)
		{
			 Show_Str(60,130,200,16,"SD Init OK",16,0);
		}
		
		MountSD();
		
		while(font_init()) //����ֿ�
		{   
			LCD_Clear(WHITE);		   	//����
			POINT_COLOR=RED;			//��������Ϊ��ɫ	   	   	  
			LCD_ShowString(60,50,200,16,16,"M3S STM32");
																
			LCD_ShowString(60,70,200,16,16,"SD Card OK");
			LCD_ShowString(60,90,200,16,16,"Font Updating...");
			key=update_font(20,110,16,0);//��SD������
			while(key)//����ʧ��		
			{			 		  
				LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
				delay_nms(200);
				LCD_Fill(20,110,200+20,110+16,WHITE);
				delay_nms(200);		       
			} 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Success!");
			delay_nms(1500);	
			LCD_Clear(WHITE);//����	       
		}  
	
		POINT_COLOR=RED;      
		Show_Str(60,50,200,16,"�ֿ�������",16,0);				    	 	
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
