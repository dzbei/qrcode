#include "getimage.h"

static char *pGetJPGImageUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Manbu/Screen/QRCode/\"";

static FRESULT res;  
static FILINFO finfo;
static FIL fsrc;

//HttpGetQRCodeList��ʱ������ ���ܴ���   ��ȡָ����JPGͼƬ  ��֤ͼƬ���ص���������  ����Ҫjson���� ֱ�ӱ���ͼƬ
//���� +HTTPREAD:8129   ���ŷ���ȫ��ͼƬ����
void Task_HttpGetJGPImage(void *param)
{
	  char picindex=0;
		u16 i = 0;
    u32 real_write_data = 0;
	
		memset((char *)buf, 0, 128);
	
		//20160522 start 
		LCD_Clear(WHITE);	

		POINT_COLOR=RED;
		Show_Str(60,150,200,16,"ϵͳ���ڻ�ȡͼƬ",16,0);
		//end
	
		//g_cJGPMsgIndex ������С
		GetJPGMsg();//�ȴ�jpg.txt�ļ�ȡ��ͼƬ���ݵ���������
	
		SendATCommand("AT+HTTPINIT","OK");
			
		//����g_stJGPMsg[picindex] ��С��ȡ����
		for(picindex=0; (picindex<g_cJGPMsgIndex); )
		{
				i  = sprintf( buf,"%s", pGetJPGImageUrl);
				i += sprintf( buf+i-1, "%s\"", g_stJGPMsg[picindex].aName);//����ȫ�������ֵ�����ж�ȡ
	
#ifdef DEBUG_APP_TASK		
				printf("\r\n");
				printf("jpg buf:%s\r\n",buf);
#endif				
			
				SendATCommand(buf, "OK");
			
#ifdef DEBUG_APP_TASK		
				printf("AT+HTTPURL\r\n");
#endif					
			
				//�������ݴ����SIM���ڲ�
				SendATCommand("AT+HTTPACTION=0","OK");//ȡ������  

#ifdef DEBUG_APP_TASK						
				printf("AT+HTTPACTION\r\n");
#endif	
				delay_nms(HTTP_GET_LONG_DELAY);
			
				g_uRecvBufIndex = 0;
				g_cStartRecvMsg = 1;
				//�Ӵ��ڽ������ݽ���   �ں�����һֱ�ȴ��������ݷ�����ɲż���
				//���ڶ�ʱ�ȴ����ص�ʱ������sΪ��λ ���Ժܿ����ڵȴ�
				//����ص���һ���ʱ���������Ѿ�����	
				//���ݳ��ȹ̶���1000-9999֮��
				UART2_SendString("AT+HTTPREAD=0,10240");   
				UART2_SendLR();
						
#ifdef DEBUG_APP_TASK						
				printf("AT+HTTPREAD\r\n");
#endif	

				delay_nms(HTTP_GET_SHORT_DELAY);
				//8153  8129 + 14 + 4 + 6
				//ֱ�Ӱ�RecvBuf������д�뵽sd��ͼƬ��
			
#ifdef DEBUG_APP_TASK						
				printf("g_uRecvBufIndex:%d\n", g_uRecvBufIndex);
#endif

				if(g_uRecvBufIndex > 5000 && g_uRecvBufIndex < 10000)
				{
						f_unlink(g_stJGPMsg[picindex].aName);

						if(FR_OK == f_open(&fsrc, g_stJGPMsg[picindex].aName, FA_CREATE_NEW | FA_WRITE))
						{
							
#ifdef DEBUG_APP_TASK						
								printf("����ͼƬ�ļ��ɹ�\r\n");
#endif
								f_write(&fsrc, &RecvBuf[18], g_uRecvBufIndex-6, &real_write_data);
						}
						
						f_close(&fsrc);
						
						picindex++;		
						
						//start 20160522
						sprintf( buf,"%s%d", "�����%",picindex*10);	
						Show_Str(60,170,200,16,buf,16,0);
						//end
				}
				
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}
		
		SendATCommand("AT+HTTPTERM","OK");
				
		LCD_Clear(WHITE);	
		
		g_stTaskStatus = TASK_IDLE;
				
		Start_ALLShowTask();  //�������е���ʾ����
}

//�ϵ���߸������ݺ����»�ȡ��ʾͼƬ����Ϣ
void GetJPGMsg(void)
{
	u8 i = 0;
	u32 temp = 0;

	memset(buf, 0, 50);
	
	g_cJGPMsgIndex = 0; //ֻ��������һ����������
	
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/jpg/jpg", i,".txt");
		//�ϵ��ȡ��Ҫ��ʾ��ͼƬ��Ϣ
		res = f_stat(buf, &finfo); //�ж��ļ��Ƿ����
		//˵�������ʾͼƬ��Ϣ���ļ��Ѿ�����
		if(res == FR_OK)
		{
			//����˳��ȡ��ͼƬ������7�ֽ�(��001.jpg)����ʾʱ��4�ֽ�(���㲹��0001)������һ�ż��ʱ��4�ֽ�(���㲹��0001)
			res = f_open(&fsrc, buf, FA_READ);
			
			if(FR_OK == res) 
			{
					//ÿ�ζ�ȡ15�ֽ�����              
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aName, 7, &temp);
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aShowTime, 4, &temp);
					f_read(&fsrc, &g_stJGPMsg[g_cJGPMsgIndex].aLastTime, 4, &temp);
				
#ifdef DEBUG_APP_TASK						
					printf("��%d��JPG\n", i);
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
	//		i = 10;//�˳�ѭ��
		}
	}
}
