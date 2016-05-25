#include "getqrcodelist.h"

static char *pGetQRCodeUrl = "AT+HTTPPARA=\"URL\",\"http://dianziping.taotaojin.cc/Screen/Screen/getQRCode/id/\"";

//24Сʱ��ʱ����      ����ȥ��������ȡ��ά��ͼƬ����  ��֤�Ƿ�ÿ�ζ�Ҫ���³�ʼ��HTTP   JSON���� ����ͼƬ�����������ĵ���
//{"result":{"id":2,"name":"005.jpg","sort":2,"showTime":8,"lastTime":2,"status":1,"cycleTime":86400},"code":10000,"message":"ok","sign":2147483647}
void Task_HttpGetQRCodeList(void *param)
{
		char picindex=0;
		u16 i=0;
		u8 flag = 1;
	
		memset(buf, 0, 128);
	
		g_stTaskStatus = TASK_UPDATE_QRCODE;
				
	//	Kill_ALLShowTask();//�ر����е���ʾ����

		LCD_Clear(WHITE);	

		POINT_COLOR=RED;
		Show_Str(60,150,200,16,"ϵͳ���ڸ�������",16,0);	

#ifdef DEBUG_APP_TASK		
		printf("Enter HttpGetQRCodeList\r\n");
#endif		
		
	#if 1
		SendATCommand("AT+HTTPINIT","OK");
			
		//�Ȱ� 0:/jpg/jpg Ŀ¼�µ������ļ�ɾ��  ����GetJPGMsgȡ��������
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
				//��֤�Ƿ���Ҫ���³�ʼ��  ����Ҫ���³�ʼ��
				SendATCommand(buf, "OK");
				
				//�������ݴ����SIM���ڲ�
			  //RET_SEND_CMD_INTERVAL �Ӵ����
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
				//���ݳ��ȹ̶���100-999֮��
				UART2_SendString("AT+HTTPREAD=0,1024");   //��ѯ�ȴ�ģ��ע��ɹ�
				UART2_SendLR();
						
				delay_nms(HTTP_GET_SHORT_DELAY);

#ifdef DEBUG_APP_TASK						
				printf("g_uRecvBufIndex:%d\r\n", g_uRecvBufIndex); //������յ������ݳ���С��100 ��ô�ͱ�ʾ�ô���Ϣ��ȡʧ�� ���»�ȡ
#endif			

				//������ݽ��ճɹ�
				if(g_uRecvBufIndex > 99 && g_uRecvBufIndex < JSON_BUF_LENGTH)
				{
						//��Ч���ݵ���ʼ  ��������ǰ�����4���ո� min=13+4      �������4������ok  max=g_uRecvBufIndex-6
						//��ӡ���н��յ�������   +HTTPREAD:146  - ok      ��ӡȫ�����յ��������ų��ո�
						for(i = 17; i < g_uRecvBufIndex-6; i++)
						{
								JsonBuf[i-17]=RecvBuf[i];
						}
						
						//��ʹ�����һ�����ݳ���ҲҪ��100��999֮��
						flag = ParseQRCodeJson(JsonBuf);
						
						//����������һ��
						picindex++;
						
						//�����ɽ�����ʾ  20160522 start
						sprintf( buf,"%s%d", "�����%",picindex*10);	
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
