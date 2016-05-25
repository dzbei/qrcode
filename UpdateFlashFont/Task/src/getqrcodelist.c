#include "getqrcodelist.h"

static char *pGetQRCodeUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Screen/Screen/getQRCode/id/\"";

//24小时定时触发      连续去服务器获取二维码图片数据  验证是否每次都要重新初始化HTTP   JSON解析 保存图片参数到各个文档中
//{"result":{"id":2,"name":"005.jpg","sort":2,"showTime":8,"lastTime":2,"status":1,"cycleTime":86400},"code":10000,"message":"ok","sign":2147483647}
void Task_HttpGetQRCodeList(void *param)
{
		char picindex=0;
		u16 i=0;
		u8 flag = 1;
	
		memset(buf, 0, 128);
	
		g_stTaskStatus = TASK_UPDATE_QRCODE;
				
	//	Kill_ALLShowTask();//关闭所有的显示任务

		LCD_Clear(WHITE);	

		POINT_COLOR=RED;
		Show_Str(60,150,200,16,"系统正在更新数据",16,0);	

#ifdef DEBUG_APP_TASK		
		printf("Enter HttpGetQRCodeList\r\n");
#endif		
		
	#if 1
		SendATCommand("AT+HTTPINIT","OK");
			
		//先把 0:/jpg/jpg 目录下的所有文件删除  避免GetJPGMsg取到旧数据
		for(i=0;i<10;i++)
		{
			sprintf( buf,"%s%d%s", "0:/jpg/jpg", i,".txt");
			f_unlink(buf);
		}

		do
		{				
				i = sprintf( buf,"%s", pGetQRCodeUrl);		
				i += sprintf( buf+i-1, "%s/sort/%d\"", g_aPhoneNum, picindex);
		
#ifdef DEBUG_APP_TASK		
				printf("\r\n");
				printf("%s\n", buf);
#endif	
				//验证是否需要重新初始化  不需要重新初始化
				SendATCommand(buf, "OK");
				
				//读出数据存放在SIM卡内部
			  //RET_SEND_CMD_INTERVAL 加大控制
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
				//数据长度固定在100-999之间
				UART2_SendString("AT+HTTPREAD=0,1024");   //查询等待模块注册成功
				UART2_SendLR();
						
				delay_nms(HTTP_GET_SHORT_DELAY);

#ifdef DEBUG_APP_TASK						
				printf("g_uRecvBufIndex:%d\r\n", g_uRecvBufIndex); //如果接收到的数据长度小于100 那么就表示该次信息获取失败 重新获取
#endif			

				//如果数据接收成功
				if(g_uRecvBufIndex > 99 && g_uRecvBufIndex < JSON_BUF_LENGTH)
				{
						//有效数据的起始  返回数据前面存在4个空格 min=13+4      后面存在4个加上ok  max=g_uRecvBufIndex-6
						//打印所有接收到的数据   +HTTPREAD:146  - ok      打印全部接收到的数据排除空格
						for(i = 17; i < g_uRecvBufIndex-6; i++)
						{
								JsonBuf[i-17]=RecvBuf[i];
						}
						
						//即使是最后一个数据长度也要在100到999之间
						flag = ParseQRCodeJson(JsonBuf);
						
						//继续接收下一个
						picindex++;
						
						//添加完成进度显示  20160522 start
						sprintf( buf,"%s%d", "已完成%",picindex*10);	
						Show_Str(60,170,200,16,buf,16,0);
						//end 
				}
							
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}while((flag == 1) && (picindex<10));
		
		SendATCommand("AT+HTTPTERM","OK");
		#endif
						
		TimerCancel(g_tHttpGetJGPImage);
		g_tHttpGetJGPImage = TimerStart((uint32_t)3000,	/*[ms]*/
									Task_HttpGetJGPImage, 					/*callback function*/
									NULL); 							/* parameter */
}
