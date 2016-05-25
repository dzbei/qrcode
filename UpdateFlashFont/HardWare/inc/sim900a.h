#ifndef __SIM900A_H
#define __SIM900A_H
#include "stm32f10x.h"

#define GET_QRCODE_LIST			1
#define GET_SCAN_CODE_LIST 	2
#define GET_JGP_IMAGE 			3

void WaitCREG(void);
void SetATE0(void);
void HttpInit(void);
void HttpGetQRCodeList(void *param);
void HttpGetScanCodeList(void);
void HttpGetJGPImage(char *p_pImgName);
void SendATCommand(char *p_pSendMsg,char *p_pRetTargetMsg);
#endif
