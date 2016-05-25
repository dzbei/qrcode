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

u32 g_uRetSendCmdInterval = 2000;//ms 默认

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* 函数名 : Second_AT_Command
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void SendATCommand(char *p_pSendMsg,char *p_pRetTargetMsg)  
{
	u8 i = 0;
	char *l_pTempSendMsg = p_pSendMsg;
	u32 l_uNow = hwclock();//ms
	u8 l_cSendFlag = 1;
		
	CLR_Buf2();               //清除串口2的接收缓存
	//一直等待预计的命令返回
	while(i == 0)                    
	{
		//查询模块返回值是否包含指定的返回字符串
		//如果有则表明命令执行完成
		//如果没有则超时重发
		if(!Find(p_pRetTargetMsg)) 
		{		
			//在等待的过程中会一直有数据进入
			//如果超时重发                   使用全局变量来存放RET_SEND_CMD_INTERVAL值 方便针对每一个情况进行修改
			 if(hwclock() > (l_uNow + g_uRetSendCmdInterval))
			 {
						l_cSendFlag = 1;
			 }
			 
			//如果在wait_time的时间内没有数据返回 那么就继续发送命令
			if(l_cSendFlag == 1)   //获取当前时间
			{
					p_pSendMsg = l_pTempSendMsg;							//将字符串地址给b
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
* 函数名 : Wait_CREG
* 描述   : 等待模块注册成功
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void WaitCREG(void)
{
	u8 i;
	u8 k;
	i = 0;
  while(i == 0)        			
	{
		CLR_Buf2();        
		UART2_SendString("AT+CREG?");   //查询等待模块注册成功
		UART2_SendLR();
		
		delay_nms(5000);  						
	    
		for(k=0;k<CMD_BUF_LENGTH;k++)      			
    	{
			if(Uart2Buf[k] == ':')
			{
				if((Uart2Buf[k+4] == '1')||(Uart2Buf[k+4] == '5'))   //说明注册成功
				{
					i = 1;
				  break;
				}
			}
		}
		
		Show_Str(60,130,200,16,"正在注册网络",16,0);
#ifdef DEBUG_APP_TASK		
		printf("Registering ......\n");
#endif
	}
}

/*******************************************************************************
* 函数名 : Set_ATE0
* 描述   : 取消回显
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void SetATE0(void)
{
	SendATCommand("ATE0","OK");								//取消回显	
}

void HttpInit(void)
{
		SendATCommand("AT+CGATT=1","OK");  
		
		SendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"","OK"); 
	  
		SendATCommand("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK");
	  
		SendATCommand("AT+SAPBR=1,1","OK");
}


