#include "sim900a.h"
#include "string.h"
#include "usart.h"
#include "delay.h"
#include "cjsonutil.h"
#include "libtimer.h"
#include "timer.h"
#include "app_task.h"

extern char Uart2Buf[];
extern char *p1;

u32 g_uRetSendCmdInterval = 2000;//ms Ĭ��

/*******************************************************************************
* ������ : Find
* ����   : �жϻ������Ƿ���ָ�����ַ���
* ����   : 
* ���   : 
* ����   : unsigned char:1 �ҵ�ָ���ַ���0 δ�ҵ�ָ���ַ� 
* ע��   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* ������ : Second_AT_Command
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢���͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void SendATCommand(char *p_pSendMsg,char *p_pRetTargetMsg)  
{
	u8 i = 0;
	char *l_pTempSendMsg = p_pSendMsg;
	u32 l_uNow = hwclock();//ms
	u8 l_cSendFlag = 1;
		
	CLR_Buf2();               //�������2�Ľ��ջ���
	//һֱ�ȴ�Ԥ�Ƶ������
	while(i == 0)                    
	{
		//��ѯģ�鷵��ֵ�Ƿ����ָ���ķ����ַ���
		//��������������ִ�����
		//���û����ʱ�ط�
		if(!Find(p_pRetTargetMsg)) 
		{		
			//�ڵȴ��Ĺ����л�һֱ�����ݽ���
			//�����ʱ�ط�                   ʹ��ȫ�ֱ��������RET_SEND_CMD_INTERVALֵ �������ÿһ����������޸�
			 if(hwclock() > (l_uNow + g_uRetSendCmdInterval))
			 {
						l_cSendFlag = 1;
			 }
			 
			//�����wait_time��ʱ����û�����ݷ��� ��ô�ͼ�����������
			if(l_cSendFlag == 1)   //��ȡ��ǰʱ��
			{
					p_pSendMsg = l_pTempSendMsg;							//���ַ�����ַ��b
					for (; *p_pSendMsg!='\0';p_pSendMsg++)
					{
						while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
						USART_SendData(USART2,*p_pSendMsg);//UART2_SendData(*b);
					}
					UART2_SendLR();	

					l_cSendFlag = 0;
		   }
    }
 	  else
		{
			i = 1;
		}
	}
	
	CLR_Buf2();	
}

/*******************************************************************************
* ������ : Wait_CREG
* ����   : �ȴ�ģ��ע��ɹ�
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void WaitCREG(void)
{
	u8 i;
	u8 k;
	i = 0;
  while(i == 0)        			
	{
		CLR_Buf2();        
		UART2_SendString("AT+CREG?");   //��ѯ�ȴ�ģ��ע��ɹ�
		UART2_SendLR();
		
		delay_nms(5000);  						
	    
		for(k=0;k<CMD_BUF_LENGTH;k++)      			
    	{
			if(Uart2Buf[k] == ':')
			{
				if((Uart2Buf[k+4] == '1')||(Uart2Buf[k+4] == '5'))   //˵��ע��ɹ�
				{
					i = 1;
				  break;
				}
			}
		}
		
		Show_Str(60,130,200,16,"����ע������",16,0);
#ifdef DEBUG_APP_TASK		
		printf("Registering ......\n");
#endif
	}
}

/*******************************************************************************
* ������ : Set_ATE0
* ����   : ȡ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void SetATE0(void)
{
	SendATCommand("ATE0","OK");								//ȡ������	
}

void HttpInit(void)
{
		SendATCommand("AT+CGATT=1","OK");  
		
		SendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"","OK"); 
	  
		SendATCommand("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK");
	  
		SendATCommand("AT+SAPBR=1,1","OK");
}


