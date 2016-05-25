#include "getversion.h"

static char *pGetTimeIntervalUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Screen/Screen/updatetime/id/18648957541\"";
static FIL fsrc;
//��������ʱ���ȡһ�� ���涨ʱ3Сʱ��ȡһ��
void Task_HttpGetVersion(void *param) 
{
		u8 i=0,flag=1;
	
		g_stTaskStatus = TASK_UPDATE_VERSION;
	
		Kill_ALLShowTask();
	
		LCD_Clear(WHITE);	
	
		POINT_COLOR=RED;
	
		Show_Str(60,130,200,16,"���ڸ��°汾��",16,0);	
	
		SendATCommand("AT+HTTPINIT","OK");
	
		SendATCommand(pGetTimeIntervalUrl, "OK");
	
		do
		{
				//�������ݴ����SIM���ڲ�
				SendATCommand("AT+HTTPACTION=0","OK");//ȡ������

				delay_nms(HTTP_GET_LONG_DELAY);

				g_uRecvBufIndex = 0;
				g_cStartRecvMsg = 1;
				//�Ӵ��ڽ������ݽ���   �ں�����һֱ�ȴ��������ݷ�����ɲż���
				//���ڶ�ʱ�ȴ����ص�ʱ������sΪ��λ ���Ժܿ����ڵȴ�
				//����ص���һ���ʱ���������Ѿ�����	
				//���ݳ��ȹ̶���100-999֮��
				UART2_SendString("AT+HTTPREAD=0,1024");   //��ѯ�ȴ�ģ��ע��ɹ�
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
		
		//����汾�Ų�һ��
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
		else //����汾������һ��һ��  3Сʱ��ȡһ�ΰ汾��
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
								
				//��ȡ��Ҫ��ʾ�Ķ�ά��ͼƬ��Ϣ  ����Task_JGPShow����
				GetJPGMsg(); //Task_HttpGetQRCodeList�����ȡ  
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
	
		//ֱ�����ж��ļ��Ƿ���� �����������ô��ֱ��ɾ��ԭ���ļ������½�д��
		if(FR_OK == f_open(&fsrc, "0:/version/version.txt", FA_READ))
		{
			f_read(&fsrc, &version, 4, &temp);
		}
		f_close(&fsrc);
	
		temp = (((u32)(version[0]))<<24) + (((u32)(version[1]))<<16) + (((u32)(version[2]))<<8) + version[3];
		
		//g_uVersion �ӷ�������ȡ
		
		//version.txt��ʼֵΪ0x1000
		if(temp != g_uVersion)  //�汾�Ų�һ���Ͱ�����̵�ʱ�������и��� 
		{
				f_unlink("0:/version/version.txt"); //ɾ��ԭ�����ļ�
				
				if(FR_OK == f_open(&fsrc, "0:/version/version.txt", FA_CREATE_NEW | FA_WRITE))
				{
					version[0]  = g_uVersion>>24;
					version[1]  = (g_uVersion>>16)&0xff;
					version[2]  = (g_uVersion>>8)&0xff;
					version[3]  = g_uVersion&0xff;
					//���µİ汾��д��  �ж��Ƿ�д��ɹ�
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
