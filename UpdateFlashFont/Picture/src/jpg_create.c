#include "jpg_create.h"
#include "lcd.h"

/* Private functions ---------------------------------------------------------*/
FATFS fs;            // Work area (file system object) for logical drive
FIL fsrc;
     
//判断文件是否存在
FRESULT IsFileExist(const char *path)
{
		FRESULT res;  
		FILINFO finfo;
	
		res = f_stat(path, &finfo);
	
    return res;
}

void MountSD(void)
{
    disk_initialize(0);
    f_mount(&fs, "", 1);
}

FRESULT CreateJPGFile(void)
{
    FRESULT res;
    u32 real_write_data = 0;
    u16 real_data_len = 0;

		//if(IsFileExist("test.jpg") != FR_OK)
		{
			f_unlink("test.jpg");
			
			res = f_open(&fsrc, "test.jpg", FA_CREATE_NEW | FA_WRITE | FA_READ);

			if(FR_OK == res) 
			{
					printf("Create test.jpg OK \n");	
			}

		//	real_data_len = RealRecvBufLength - 6;
			//5786
			printf("real_data_len:%d \n", real_data_len);
			
		//	res = f_write(&fsrc, &USART_RecvMsg.RecvBuf[2], real_data_len, &real_write_data);
			
			f_close(&fsrc);
		}

    return res;
}
						
