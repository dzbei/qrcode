#include "app_task.h"

const u8 *g_pHead="微信号:";
//http://dianziping.taotaojin.cc/Screen/Screen/getQRCode/id/18648957541/qid/0
//{"result":{"id":1,"name":"001.jpg","sort":0,"showTime":10,"lastTime":1,"status":1,"cycleTime":86400},"code":10000,"message":"ok","sign":1736912233}
//本卡号码 18648957541  按键按下是使用该url
//http://dianziping.taotaojin.cc/Screen/Screen/check/id/18648957541/lid/0
//{"result":{"id":1,"name":"aa","datetime":"2016-04-25 16:38:54","status":1},"code":10000,"message":"ok","sign":901498365}

//直接从SD卡的文件里面读取出来
char g_aPhoneNum[]={"18648957541"};

TimerHandleT g_tHttpGetQRCode = INVALID_TIMER;
TimerHandleT g_tHttpGetJGPImage = INVALID_TIMER;
TimerHandleT g_tHttpGetVersion = INVALID_TIMER;
TimerHandleT g_tJPGShow = INVALID_TIMER;
TimerHandleT g_tNameShow = INVALID_TIMER;

u8 g_cJGPMsgIndex = 0;

JGP_MSG_TypeDef g_stJGPMsg[10];

SCAN_CODE_TypeDef g_stScanCode[10];

u8 g_bNameShowTaskRestart = 1;//如果任务重新启动 那么清除所有的静态变量
u8 g_bJGPShowTaskRestart = 1;

u32 g_uJPGShowInterval = 0;

char JsonBuf[JSON_BUF_LENGTH]; //4个task共用

u8 g_aLcdShowBuf[200];

TaskStatus_TypeDef g_stTaskStatus;

char buf[128];//4个task共用的buf

void CLRJsonBuf(void)
{
	u16 k;
	for(k=0;k<JSON_BUF_LENGTH;k++)      //将缓存内容清零
	{
		JsonBuf[k] = 0x00;
	}
}

void Start_ALLShowTask(void)
{
		g_bNameShowTaskRestart = 1;	
	  g_bJGPShowTaskRestart = 1;
	
		if(g_tNameShow == INVALID_TIMER)
		{
			g_tNameShow = TimerStart((uint32_t)2000,	/*[ms]*/
									Task_NameShow, 					/*callback function*/
									NULL); 							/* parameter */
		}
		
		if(g_tJPGShow == INVALID_TIMER)
		{
			g_tJPGShow = TimerStart((uint32_t)3000,	/*[ms]*/
										Task_JGPShow, 					/*callback function*/
										NULL); 							/* parameter */
		}
}

void Kill_ALLShowTask(void)
{
		//取消轮询显示任务
		if(g_tNameShow != INVALID_TIMER)
		{
			TimerCancel( g_tNameShow );
			g_tNameShow = INVALID_TIMER;
		}
		
		if(g_tJPGShow != INVALID_TIMER)
		{
			TimerCancel( g_tJPGShow );
			g_tJPGShow = INVALID_TIMER;
		}
}

void Task_NameShow(void *param)
{
	static u8 s_uIndex=0;//i->s_uIndex
	static u8 s_uLen = 0;//len->s_uLen
	u8 j=0,k=0,templen=0;//增加templen
	
	TimerCancel(g_tNameShow);
	g_tNameShow = TimerStart((uint32_t)1000,	/*[ms]*/
									Task_NameShow, 					/*callback function*/
									NULL); 							/* parameter */

	if(g_bNameShowTaskRestart)
	{
		g_bNameShowTaskRestart = 0;
		s_uIndex = 0;

		memset(g_aLcdShowBuf,0,200);

		s_uLen=strlen(g_pHead);
		memcpy(g_aLcdShowBuf, g_pHead, s_uLen);
	
		for(j=0; j<10; j++)
		{
				templen = strlen(g_stScanCode[j].aName);
				for(k=0; k<templen; k++)
				{
					g_aLcdShowBuf[s_uLen+k]=g_stScanCode[j].aName[k];
				}
				
				g_aLcdShowBuf[s_uLen+templen] = ',';
				s_uLen	+= templen+1;
		}
	}
	
	//一行可以显示30个字符
	if((s_uIndex+30)>s_uLen)
	{
			s_uIndex = 0;
	}
	
	//清一次屏
	LCD_Fill(0,0,240,16,WHITE);
	
	POINT_COLOR=BLUE;
	Show_Str(0,0,240,16,(g_aLcdShowBuf+s_uIndex),16,0);	
	s_uIndex=s_uIndex+2;
}

void Task_JGPShow(void *param)
{
	static u8 picindex = 0;
	static u8 showflag = 1;
	char tempbuf[20];
	
	memset(tempbuf, 0, 20);
	
	if(g_bJGPShowTaskRestart)
	{
		g_bJGPShowTaskRestart = 0;
		picindex = 0;
		showflag = 1;
	}
	
	if(showflag == 1)
	{  //
		showflag = 0;
		g_uJPGShowInterval = g_stJGPMsg[picindex].aShowTime[3] * 1000;

		sprintf( tempbuf,"0:/%s", g_stJGPMsg[picindex].aName);

#ifdef DEBUG_APP_TASK		
		printf("jpg show time:%d\r\n", g_stJGPMsg[picindex].aShowTime[3]);
		printf("jpgname:%s\r\n", tempbuf);
#endif
		
		//显示起点为(0,20) 避免覆盖显示的文字
    ai_load_picfile(tempbuf,0,20,lcddev.width,lcddev.height-20,1);	
	}
	else
	{
		showflag = 1;
		g_uJPGShowInterval = g_stJGPMsg[picindex].aLastTime[3] * 1000;
		
		//把显示图片的区域清零
		LCD_Fill(0,20,240,320,WHITE);
		
		if(picindex < (g_cJGPMsgIndex-1))
		{
				picindex++;
		}
		else
		{
				picindex = 0;
		}
	}

	TimerCancel(g_tJPGShow);
	g_tJPGShow = TimerStart((uint32_t)g_uJPGShowInterval,	/*[ms]*/
									Task_JGPShow, 					/*callback function*/
									NULL); 							/* parameter */
}

