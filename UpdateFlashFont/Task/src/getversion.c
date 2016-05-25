#include "getversion.h"

static char *pGetTimeIntervalUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Screen/Screen/updatetime/id/18648957541\"";
static FIL fsrc;
//刚启动的时候获取一次 后面定时3小时获取一次
void Task_HttpGetVersion(void *param) 
{
		u8 i=0,flag=1;
	
		g_stTaskStatus = TASK_UPDATE_VERSION;
	
		Kill_ALLShowTask();
	
		LCD_Clear(WHITE);	
	
		POINT_COLOR=RED;
	
		Show_Str(60,130,200,16,"正在更新版本号",16,0);	
	
		SendATCommand("AT+HTTPINIT","OK");
	
		SendATCommand(pGetTimeIntervalUrl, "OK");
	
		do
		{
				//读出数据存放在SIM卡内部
				SendATCommand("AT+HTTPACTION=0","OK");//取出数据

				delay_nms(HTTP_GET_LONG_DELAY);

				g_uRecvBufIndex = 0;
				g_cStartRecvMsg = 1;
				//从串口进行数据接收   在函数会一直等待到有数据返回完成才继续
				//由于定时等待返回的时间间隔以s为单位 所以很可能在等待
				//命令返回的那一秒的时间里数据已经返回	
				//数据长度固定在100-999之间
				UART2_SendString("AT+HTTPREAD=0,1024");   //查询等待模块注册成功
				UART2_SendLR();
						
				delay_nms(HTTP_GET_SHORT_DELAY);
			
#ifdef DEBUG_APP_TASK						
				printf("g_uRecvBufIndex:%d\n", g_uRecvBufIndex);
#endif			

				if(g_uRecvBufIndex > 50 && g_uRecvBufIndex < 150)
				{
						for(i = 16; i < g_uRecvBufIndex-6; i++)
						{
								JsonBuf[i-16]=RecvBuf[i];
						}
						
						ParseVersionJson(JsonBuf);
						
						flag = 0;
				}
							
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}while(flag);
		
		SendATCommand("AT+HTTPTERM","OK");
		
		//如果版本号不一致
		if(IsVersionUpate())
		{
#ifdef DEBUG_APP_TASK					
				printf("Version diff\r\n");
#endif	
				if(g_tHttpGetQRCode == INVALID_TIMER)
				{
					g_tHttpGetQRCode = TimerStart((uint32_t)g_uGetQRCodeQuickTime,	/*[ms]*/
											Task_HttpGetQRCodeList, 					/*callback function*/
											NULL); 							/* parameter */
				}
		}
		else //如果版本号与上一次一样  3小时获取一次版本号
		{
#ifdef DEBUG_APP_TASK					
				printf("Version Same\r\n");
#endif	
				if(g_tHttpGetVersion == INVALID_TIMER)
				{					
					g_tHttpGetVersion = TimerStart((uint32_t)10800000,	/*[ms]*/
											Task_HttpGetVersion, 					/*callback function*/
											NULL); 							/* parameter */
				}
				
				g_stTaskStatus = TASK_IDLE;
								
				//获取需要显示的二维码图片信息  用于Task_JGPShow任务
				GetJPGMsg(); //Task_HttpGetQRCodeList任务获取  
#ifdef DEBUG_APP_TASK				

				printf("Start_ALLShowTask Time:%d\r\n", hwclock());
#endif				
				Start_ALLShowTask();
		}
}

u8 IsVersionUpate(void)
{
		u32 temp=0;
		u8 version[4];
		u8 ret = FALSE;
	
		memset(version, 0, 4);
	
		//直接先判断文件是否存在 如果不存在那么就直接删除原来文件重新新建写入
		if(FR_OK == f_open(&fsrc, "0:/version/version.txt", FA_READ))
		{
			f_read(&fsrc, &version, 4, &temp);
		}
		f_close(&fsrc);
	
		temp = (((u32)(version[0]))<<24) + (((u32)(version[1]))<<16) + (((u32)(version[2]))<<8) + version[3];
		
		//g_uVersion 从服务器获取
		
		//version.txt初始值为0x1000
		if(temp != g_uVersion)  //版本号不一样就按照最短的时间来进行更新 
		{
				f_unlink("0:/version/version.txt"); //删除原来的文件
				
				if(FR_OK == f_open(&fsrc, "0:/version/version.txt", FA_CREATE_NEW | FA_WRITE))
				{
					version[0]  = g_uVersion>>24;
					version[1]  = (g_uVersion>>16)&0xff;
					version[2]  = (g_uVersion>>8)&0xff;
					version[3]  = g_uVersion&0xff;
					//把新的版本号写入  判断是否写入成功
					f_write(&fsrc, version, 4, &temp);
				}
				
				f_close(&fsrc);
				
				ret = TRUE;
		}
		
		return ret;
}

void GetPhoneNum(void)
{	
	u32 temp;
	
	if(FR_OK == f_open(&fsrc, "0:/phone/phone.txt", FA_READ))
	{
		f_read(&fsrc, &g_aPhoneNum, 11, &temp);
#ifdef DEBUG_APP_TASK		
		printf("phonenum:%s\r\n", g_aPhoneNum);
#endif
	}
	
	f_close(&fsrc);
}
