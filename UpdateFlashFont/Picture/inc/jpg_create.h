#ifndef __JPG_CREATE_H
#define __JPG_CREATE_H

#include "ff.h"
#include "diskio.h"
#include <stdlib.h>
#include "usart.h"

typedef struct
{
    char aName[7];
    char aShowTime[4];
		char aLastTime[4];
}JGP_MSG_TypeDef;

//只存放微信名字
typedef struct
{
    char aName[20];//微信名称不超过20个
}SCAN_CODE_TypeDef;

FRESULT IsFileExist(const char* path);
FRESULT CreateJPGFile(void);
void MountSD(void);

#endif
