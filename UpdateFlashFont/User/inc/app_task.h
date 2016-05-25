#ifndef __APP_TASK_H
#define __APP_TASK_H  

#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "timer.h"
#include "libtimer.h"
#include "lcd.h"
#include "jpg_create.h"
#include "sim900a.h"
#include "delay.h"
#include "text.h"
#include "cjsonutil.h"
#include "piclib.h"

//#define DEBUG_APP_TASK					

#define JSON_BUF_LENGTH					256

#define HTTP_GET_LONG_DELAY 		8000
#define HTTP_GET_SHORT_DELAY 		3000

typedef enum
{
	TASK_IDLE,
	TASK_UPDATE_QRCODE,
	TASK_UPDATE_NAME,
	TASK_UPDATE_VERSION
}TaskStatus_TypeDef;

extern char buf[];
extern char JsonBuf[];
extern char RecvBuf[];
extern char Uart2Buf[];
extern char g_aPhoneNum[];
extern SCAN_CODE_TypeDef g_stScanCode[];
extern JGP_MSG_TypeDef g_stJGPMsg[];
extern u8 g_cJGPMsgIndex;
extern u32 g_uVersion;
extern u8 g_cStartRecvMsg;
extern u32 g_uGetQRCodeQuickTime;
extern __IO u32 g_uRecvBufIndex;
extern TaskStatus_TypeDef g_stTaskStatus;
extern SCAN_CODE_LIST_MEM_TypeDef g_stScanCodeListMem;

extern TimerHandleT g_tHttpGetQRCode;
extern TimerHandleT g_tHttpGetJGPImage;
extern TimerHandleT g_tHttpGetVersion;
extern TimerHandleT g_tJPGShow;
extern TimerHandleT g_tNameShow;

void Task_NameShow(void *param);
void Task_HttpGetQRCodeList(void *param);
void Task_HttpGetJGPImage(void *param);
void Task_HttpGetScanCodeList(void);
void Task_JGPShow(void *param);
void GetScanCodeName(void);
void GetJPGMsg(void);
void Start_ALLShowTask(void);
void Kill_ALLShowTask(void);
void GetPhoneNum(void);
void WriteScanCodeName(char *name, unsigned int id);
void Task_HttpGetVersion(void *param);
unsigned char IsVersionUpate(void);
#endif

