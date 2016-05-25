#include "getimage.h"

static char *pGetJPGImageUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Manbu/Screen/QRCode/\"";

static FRESULT res;  
static FILINFO finfo;
static FIL fsrc;

//HttpGetQRCodeList定时触发后 才能触发   获取指定的JPG图片  验证图片返回的数据内容  不需要json解析 直接保存图片
//返回 +HTTPREAD:8129   接着返回全部图片数据
void Task_HttpGetJGPImage(void *param)
{
	  char picindex=0;
		u16 i = 0;
    u32 real_write_data = 0;
	
		memset((char *)buf, 0, 128);
	
		//20160522 start 
		LCD_Clear(WHITE);	

		POINT_COLOR=RED;
		Show_Str(60,150,200,16,"系统正在获取图片",16,0);
		//end
	
		//g_cJGPMsgIndex 定出大小
		GetJPGMsg();//先从jpg.txt文件取出图片数据到数组里面
	
		SendATCommand("AT+HTTPINIT","OK");
			
		//根据g_stJGPMsg[picindex] 大小来取数据
		for(picindex=0; (picindex<g_cJGPMsgIndex); )
		{
				i  = sprintf( buf,"%s", pGetJPGImageUrl);
				i += sprintf( buf+i-1, "%s\"", g_stJGPMsg[picindex].aName);//根据全局数组的值来进行读取
	
#ifdef DEBUG_APP_TASK		
				printf("\r\n");
				printf("jpg buf:%s\r\n",buf);
#endif				
			
				SendATCommand(buf, "OK");
			
#ifdef DEBUG_APP_TASK		
				printf("AT+HTTPURL\r\n");
#endif					
			
				//读出数据存放在SIM卡内部
				SendATCommand("AT+HTTPACTION=0","OK");//取出数据  

#ifdef DEBUG_APP_TASK						
				printf("AT+HTTPACTION\r\n");
#endif	
				delay_nms(HTTP_GET_LONG_DELAY);
			
				g_uRecvBufIndex = 0;
				g_cStartRecvMsg = 1;
				//从串口进行数据接收   在函数会一直等待到有数据返回完成才继续
				//由于定时等待返回的时间间隔以s为单位 所以很可能在等待
				//命令返回的那一秒的时间里数据已经返回	
				//数据长度固定在1000-9999之间
				UART2_SendString("AT+HTTPREAD=0,10240");   
				UART2_SendLR();
						
#ifdef DEBUG_APP_TASK						
				printf("AT+HTTPREAD\r\n");
#endif	

				delay_nms(HTTP_GET_SHORT_DELAY);
				//8153  8129 + 14 + 4 + 6
				//直接把RecvBuf的数据写入到sd卡图片中
			
#ifdef DEBUG_APP_TASK						
				printf("g_uRecvBufIndex:%d\n", g_uRecvBufIndex);
#endif

				if(g_uRecvBufIndex > 5000 && g_uRecvBufIndex < 10000)
				{
						f_unlink(g_stJGPMsg[picindex].aName);

						if(FR_OK == f_open(&fsrc, g_stJGPMsg[picindex].aName, FA_CREATE_NEW | FA_WRITE))
						{
							
#ifdef DEBUG_APP_TASK						
								printf("创建图片文件成功\r\n");
#endif
								f_write(&fsrc, &RecvBuf[18], g_uRecvBufIndex-6, &real_write_data);
						}
						
						f_close(&fsrc);
						
						picindex++;		
						
						//start 20160522
						sprintf( buf,"%s%d", "已完成%",picindex*10);	
						Show_Str(60,170,200,16,buf,16,0);
						//end
				}
				
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}
		
		SendATCommand("AT+HTTPTERM","OK");
				
		LCD_Clear(WHITE);	
		
		g_stTaskStatus = TASK_IDLE;
				
		Start_ALLShowTask();  //开启所有的显示任务
}

//上电或者更新数据后重新获取显示图片的信息
void GetJPGMsg(void)
{
	u8 i = 0;
	u32 temp = 0;

	memset(buf, 0, 50);
	
	g_cJGPMsgIndex = 0; //只有在这里一处把它清零
	
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/jpg/jpg", i,".txt");
		//上电获取需要显示的图片信息
		res = f_stat(buf, &finfo); //判断文件是否存在
		//说明存放显示图片信息的文件已经存在
		if(res == FR_OK)
		{
			//按照顺序取出图片的名字7字节(如001.jpg)、显示时长4字节(不足补零0001)、与下一张间隔时间4字节(不足补零0001)
			res = f_open(&fsrc, buf, FA_READ);
			
			if(FR_OK == res) 
			{
					//每次读取15字节数据              
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aName, 7, &temp);
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aShowTime, 4, &temp);
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aLastTime, 4, &temp);
				
#ifdef DEBUG_APP_TASK						
					printf("第%d个JPG\n", i);
					printf("name:%s\n",g_stJGPMsg[g_cJGPMsgIndex].aName);
					printf("lasttime:%d\n",g_stJGPMsg[g_cJGPMsgIndex].aShowTime[3]);
					printf("showtime:%d\n",g_stJGPMsg[g_cJGPMsgIndex].aLastTime[3]);
					printf("\r\n");
#endif
				
				  g_cJGPMsgIndex++;
			}
			
			f_close(&fsrc);
		}
		else
		{
	//		i = 10;//退出循环
		}
	}
}
