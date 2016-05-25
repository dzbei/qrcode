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
	
		//读取上一次ID值
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

//按键API  返回最新的扫描信息的用户名字   验证返回的最新的内容  JSON解析保存到全局数组
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
		Show_Str(40,130,200,16,"正在获取扫描用户信息",16,0);	
	
		SendATCommand("AT+HTTPINIT","OK");
		
		do
		{		
				lastid = GetLastID();
				//lid 使用
				i = sprintf( tempbuf,"%s", pGetScanCodeUrl);			
				i += sprintf( tempbuf+i-1, "%s/lid/%d\"", g_aPhoneNum, lastid);

		#ifdef DEBUG_APP_TASK						
					printf("ScanCodeUrl:%s\r\n", tempbuf);
		#endif
				
				SendATCommand(tempbuf, "OK");

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

				if(g_uRecvBufIndex > 99 && g_uRecvBufIndex < 1000)
				{
						//有效数据的起始  min=17        max=g_uRecvBufIndex-6
						//打印所有接收到的数据
						for(i = 17; i < g_uRecvBufIndex-6; i++)
						{
								JsonBuf[i-17]=RecvBuf[i];
						}
						
						//解析完后 保存到文件中  也把数据更新到全局数组
						if(ParseScanCodeJson(JsonBuf) == FALSE)
						{
								flag = 0;
						}
				}
							
				g_cStartRecvMsg = 0;
				g_uRecvBufIndex = 0;
		}while(flag);
		
		SendATCommand("AT+HTTPTERM","OK");
					
		//防止按键多次按下执行该函数
		g_stTaskStatus = TASK_IDLE;
		
		LCD_Clear(WHITE);	
		
		Start_ALLShowTask();
}

//把新的二维码名字写入到文件里面  配合按键API进行使用
//再保存一个最新的用户的id    0:/name/lastid.txt  每次按键API时 先读取该文件的id值
void WriteScanCodeName(char *name, u32 lastid)  //先判断需要写入多少个二维码名字
{
	u8 i = 0;
	u8 id[4];
	u32 real_write_data = 0;
	
	memset(id, 0, 4);
	memset(buf, 0, 128);
	
	//初始化预先放置10个用户的名称在里面作为初始化值
	for(i=1; i<10; i++)
	{
		memset(g_stScanCode[i-1].aName, 0, strlen(g_stScanCode[i].aName)+1);//每一个重新清0
		memcpy(g_stScanCode[i-1].aName, g_stScanCode[i].aName, strlen(g_stScanCode[i].aName)+1);
	}
	
	//复制新的名字到数组里面   来源于按键API
	memset(g_stScanCode[9].aName, 0, strlen(g_stScanCode[9].aName)+1);
	memcpy(g_stScanCode[9].aName,name, strlen(name)+1);
				
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/name/name", i,".txt");
		
		f_unlink(buf); //把之前的文件删除
		
		//重新新建
		res = f_open(&fsrc, buf, FA_CREATE_NEW | FA_WRITE);
					
		if(FR_OK == res) 
		{				
			f_write(&fsrc, g_stScanCode[i].aName, strlen(g_stScanCode[i].aName)+1, &real_write_data);
			
	#ifdef DEBUG_APP_TASK						
			printf("第%d个名字:%s\n", i, g_stScanCode[i].aName);
			printf("\r\n");
	#endif
		}
		
		f_close(&fsrc);
	}
	
	//保存最新的id值
		f_unlink("0:/name/lastid.txt");//删除
	
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

//上电获取历史的二维码扫描用户信息
void GetScanCodeName(void)
{	
	u8 i = 0;
	u32 temp = 0;
	
	memset(buf, 0, 128);
	
	//上电获取最近一次扫描的二维码信息  主要是取出扫描名字(2-16字节) 进行显示
	for(i=0; i<10; i++)
	{
		sprintf( buf,"%s%d%s", "0:/name/name", i,".txt");
	
		res = f_stat(buf, &finfo);
		
		//说明存放二维码信息的文件已经存在
		if(res == FR_OK)
		{	
			res = f_open(&fsrc, buf, FA_READ);
			
			if(FR_OK == res) 
			{
				//每次读取20字节数据
				f_read(&fsrc, &g_stScanCode[i].aName, fsrc.fsize, &temp); //20->fsrc.fszie
			}
			
			f_close(&fsrc);
		}
	}
}
