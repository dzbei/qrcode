#include "getscancodelist.h"

static char *pGetScanCodeUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Screen/Screen/check/id/\"";

static FRESULT res;  
static FILINFO finfo;
static FIL fsrc;

static u32 GetLastID(void)
{
		u32 i=0;
		u8 tempid[4];
	  u32 lastid = 0;
	
		memset(tempid, 0, 4);
	
		//��ȡ��һ��IDֵ
		res = f_stat("0:/name/lastid.txt", &finfo);
		
		if(res == FR_OK)
		{	
			res = f_open(&fsrc, "0:/name/lastid.txt", FA_READ);
			
			if(FR_OK == res) 
			{				
				f_read(&fsrc, tempid, 4, &i);
				
				if(tempid[0] == 0x31)
				{
					lastid = 0;
				}
				else
				{
					lastid = (((u32)(tempid[0]))<<24) + (((u32)(tempid[1]))<<16) + (((u32)(tempid[2]))<<8) + tempid[3];
				}
				
#ifdef DEBUG_APP_TASK						
				printf("lastid:%d\n", lastid);
				printf("\r\n");
#endif
			}
			
			f_close(&fsrc);
		}
		
		return lastid;
}	

//����API  �������µ�ɨ����Ϣ���û�����   ��֤���ص����µ�����  JSON�������浽ȫ������
void Task_HttpGetScanCodeList(void) 
{
		u32 i = 0;
		u32 lastid = 0;
		u8 flag = 1;
	
		char tempbuf[128];
	
		memset(tempbuf, 0, 128);
	
		g_stTaskStatus = TASK_UPDATE_NAME;
	
		Kill_ALLShowTask();
	
		LCD_Clear(WHITE);	
	
		POINT_COLOR=RED;
		Show_Str(40,130,200,16,"���ڻ�ȡɨ���û���Ϣ",16,0);	
	
		SendATCommand("AT+HTTPINIT","OK");
		
		do
		{		
				lastid = GetLastID();
				//lid ʹ��
				i = sprintf( tempbuf,"%s", pGetScanCodeUrl);			
				i += sprintf( tempbuf+i-1, "%s/lid/%d\"", g_aPhoneNum, lastid);

		#ifdef DEBUG_APP_TASK						
					printf("ScanCodeUrl:%s\r\n", tempbuf);
		#endif
				
				SendATCommand(tempbuf, "OK");

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

				if(g_uRecvBufIndex > 99 && g_uRecvBufIndex < 1000)
				{
						//��Ч���ݵ���ʼ  min=17        max=g_uRecvBufIndex-6
						//��ӡ���н��յ�������
						for(i = 17; i < g_uRecvBufIndex-6; i++)
						{
								JsonBuf[i-17]=RecvBuf[i];
						}
						
						//������� ���浽�ļ���  Ҳ�����ݸ��µ�ȫ������
						if(ParseScanCodeJson(JsonBuf) == FALSE)
						{
								flag = 0;
						}
				}
							
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}while(flag);
		
		SendATCommand("AT+HTTPTERM","OK");
					
		//��ֹ������ΰ���ִ�иú���
		g_stTaskStatus = TASK_IDLE;
		
		LCD_Clear(WHITE);	
		
		Start_ALLShowTask();
}

//���µĶ�ά������д�뵽�ļ�����  ��ϰ���API����ʹ��
//�ٱ���һ�����µ��û���id    0:/name/lastid.txt  ÿ�ΰ���APIʱ �ȶ�ȡ���ļ���idֵ
void WriteScanCodeName(char *name, u32 lastid)  //���ж���Ҫд����ٸ���ά������
{
	u8 i = 0;
	u8 id[4];
	u32 real_write_data = 0;
	
	memset(id, 0, 4);
	memset(buf, 0, 128);
	
	//��ʼ��Ԥ�ȷ���10���û���������������Ϊ��ʼ��ֵ
	for(i=1; i<10; i++)
	{
		memset(g_stScanCode[i-1].aName, 0, strlen(g_stScanCode[i].aName)+1);//ÿһ��������0
		memcpy(g_stScanCode[i-1].aName, g_stScanCode[i].aName, strlen(g_stScanCode[i].aName)+1);
	}
	
	//�����µ����ֵ���������   ��Դ�ڰ���API
	memset(g_stScanCode[9].aName, 0, strlen(g_stScanCode[9].aName)+1);
	memcpy(g_stScanCode[9].aName,name, strlen(name)+1);
				
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/name/name", i,".txt");
		
		f_unlink(buf); //��֮ǰ���ļ�ɾ��
		
		//�����½�
		res = f_open(&fsrc, buf, FA_CREATE_NEW | FA_WRITE);
					
		if(FR_OK == res) 
		{				
			f_write(&fsrc, g_stScanCode[i].aName, strlen(g_stScanCode[i].aName)+1, &real_write_data);
			
	#ifdef DEBUG_APP_TASK						
			printf("��%d������:%s\n", i, g_stScanCode[i].aName);
			printf("\r\n");
	#endif
		}
		
		f_close(&fsrc);
	}
	
	//�������µ�idֵ
		f_unlink("0:/name/lastid.txt");//ɾ��
	
		res = f_open(&fsrc, "0:/name/lastid.txt", FA_CREATE_NEW | FA_WRITE);
		
		if(FR_OK == res) 
		{				
			id[0]  = lastid>>24;
			id[1]  = (lastid>>16)&0xff;
			id[2]  = (lastid>>8)&0xff;
			id[3]  = lastid&0xff;
			
			f_write(&fsrc, id, 4, &real_write_data);
			
#ifdef DEBUG_APP_TASK						
			printf("lastid:%d\n", lastid);
			printf("\r\n");
#endif
		}
		
		f_close(&fsrc);
}

//�ϵ��ȡ��ʷ�Ķ�ά��ɨ���û���Ϣ
void GetScanCodeName(void)
{	
	u8 i = 0;
	u32 temp = 0;
	
	memset(buf, 0, 128);
	
	//�ϵ��ȡ���һ��ɨ��Ķ�ά����Ϣ  ��Ҫ��ȡ��ɨ������(2-16�ֽ�) ������ʾ
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/name/name", i,".txt");
	
		res = f_stat(buf, &finfo);
		
		//˵����Ŷ�ά����Ϣ���ļ��Ѿ�����
		if(res == FR_OK)
		{	
			res = f_open(&fsrc, buf, FA_READ);
			
			if(FR_OK == res) 
			{
				//ÿ�ζ�ȡ20�ֽ�����
				f_read(&fsrc, &g_stScanCode[i].aName, fsrc.fsize, &temp); //20->fsrc.fszie
			}
			
			f_close(&fsrc);
		}
	}
}
