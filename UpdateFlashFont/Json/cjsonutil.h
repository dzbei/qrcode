#ifndef __CJSONUTIL__h
#define __CJSONUTIL__h
#include "stm32f10x.h"
#include "cJSON.h"

typedef struct 
{
    u32 cID;
    char *pName;
    u8 cSort;
    u32 uShowTime;
    u32 uLastTime;
		u8 cStatus;
		u32 uCode;
}QCLIST_MEM_TypeDef;

typedef struct
{
    u32 cID;
    char *pName;
		char *datetime;
		u8 cStatus;
}SCAN_CODE_LIST_MEM_TypeDef;

void ParseQRCodeList(cJSON *pCJson);
void ParseScanCodeList(cJSON* pCJson);
u8 ParseQRCodeJson(const char *pMsg);
u8 ParseScanCodeJson(const char* pszJsonText);
u8 ParseVersionJson(const char* pszJsonText);
#endif 
