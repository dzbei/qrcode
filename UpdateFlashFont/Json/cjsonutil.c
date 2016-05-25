#include "cjsonutil.h"
#include "stdlib.h"
#include <stdarg.h>
#include "stdio.h"
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "app_task.h"
#include "crc32.h"
 
#define DEBUG_JSON  

extern __IO u8 g_bGetQRCodeEnd;

QCLIST_MEM_TypeDef g_stQCListMem;

SCAN_CODE_LIST_MEM_TypeDef g_stScanCodeListMem;

u32 g_uVersion = 0;

static char jpgnamebuf[50];

static FIL fsrc;

//��ʱAPI��������
u8 ParseQRCodeJson(const char* pszJsonText)
{
    cJSON	*root, *result;
    u32 real_write_data=0; //sign=0, 
		u8 tempbuf[15], i=0, ret=1;
 
    root= cJSON_Parse(pszJsonText);
			
    if (root)
    { 
        result = cJSON_GetObjectItem(root, "result");
        if (result)
        { 	
						g_stQCListMem.cID =  cJSON_GetObjectItem(result, "id")->valueint;
					
						//��־�ﵽ�����һ��  �������ݱ���
						if(g_stQCListMem.cID  == 10001)
						{
								ret = 0;
#ifdef DEBUG_APP_TASK	
								printf("\r\n");
								printf("��ȡ���ݽ���");  
#endif							
						}
						else
						{
							
							g_stQCListMem.pName = cJSON_GetObjectItem(result, "name")->valuestring;
							g_stQCListMem.cSort = cJSON_GetObjectItem(result, "sort")->valueint;
							g_stQCListMem.uShowTime = cJSON_GetObjectItem(result, "showTime")->valueint;//�ֽ�Ϊ4���ֽ�
							g_stQCListMem.uLastTime = cJSON_GetObjectItem(result, "lastTime")->valueint;//�ֽ�Ϊ4���ֽ�   
							g_stQCListMem.cStatus = cJSON_GetObjectItem(result, "status")->valueint;

#ifdef DEBUG_APP_TASK	
							printf("\r\n");
							printf(" cID:%d ", g_stQCListMem.cID);        
							printf(" pName:%s ", g_stQCListMem.pName);				 
							printf(" cSort:%d ", g_stQCListMem.cSort);						
							printf(" uShowTime:%d ", g_stQCListMem.uShowTime);  
							printf(" uLastTime:%d ", g_stQCListMem.uLastTime);  
							printf(" cStatus:%d ", g_stQCListMem.cStatus);
							printf(" code:%d ", g_stQCListMem.uCode); 
#endif								

							//�������ݵ�  g_stQCListMem.pName�ı�       ������ָ�����ļ� ����"0:/jpg/jpg0.txt"Ŀ¼
							//����sort��ֵ���������� ��0��ʼ ���� ���ظ�
							memset(jpgnamebuf, 0, 50);
							memset(tempbuf, 0, 15);
							
							sprintf( jpgnamebuf,"%s%d%s", "0:/jpg/jpg", g_stQCListMem.cSort,".txt");

#ifdef DEBUG_APP_TASK	
							printf("\r\n");
							printf("jpgnamebuf:%s\r\n", jpgnamebuf);
#endif
				
							//ɾ��ԭ�����ļ�
						//	f_unlink(buf);  

							if(FR_OK == f_open(&fsrc, jpgnamebuf, FA_CREATE_NEW | FA_WRITE))
							{
#ifdef DEBUG_APP_TASK	
								printf("FA_CREATE_NEW Success\r\n");
#endif
													
									for(i=0; i<7; i++)
									{
										tempbuf[i] = g_stQCListMem.pName[i];
									}
									
									tempbuf[7]  = g_stQCListMem.uShowTime>>24;
									tempbuf[8]  = (g_stQCListMem.uShowTime>>16)&0xff;
									tempbuf[9]  = (g_stQCListMem.uShowTime>>8)&0xff;
									tempbuf[10] = g_stQCListMem.uShowTime&0xff;
									
									tempbuf[11]  = g_stQCListMem.uLastTime>>24;
									tempbuf[12]  = (g_stQCListMem.uLastTime>>16)&0xff;
									tempbuf[13]  = (g_stQCListMem.uLastTime>>8)&0xff;
									tempbuf[14] = g_stQCListMem.uLastTime&0xff;
									
									f_write(&fsrc, tempbuf, 15, &real_write_data);

									f_close(&fsrc);
							}
							else
							{
#ifdef DEBUG_APP_TASK	
								printf("FA_CREATE_NEW failed\r\n");
#endif							
							}
						}
        }
        
        cJSON_Delete(root);
    }
 
    return ret;
}

//��ťAPI��������
////{"result":{"id":1,"name":"aa","datetime":"2016-04-25 16:38:54","status":1},"code":10000,"message":"ok","sign":901498365}
u8 ParseScanCodeJson(const char* pszJsonText)
{
    cJSON	*root, *result;
    u32 code=0;
		u8 ret = FALSE;
 
    root= cJSON_Parse(pszJsonText);
 
    if (root)
    { 
				code = cJSON_GetObjectItem(root, "code")->valueint;
				
				if(code != 10001)
				{
						result = cJSON_GetObjectItem(root, "result");
										
						if (result)
						{ 
								g_stScanCodeListMem.cID =  cJSON_GetObjectItem(result, "id")->valueint;
								g_stScanCodeListMem.pName = cJSON_GetObjectItem(result, "name")->valuestring;
								g_stScanCodeListMem.datetime = cJSON_GetObjectItem(result, "datetime")->valuestring;   
								g_stScanCodeListMem.cStatus = cJSON_GetObjectItem(result, "status")->valueint;
								
							//����ɨ���û�������  ��idֵҲ��Ϊһ����������
								WriteScanCodeName(g_stScanCodeListMem.pName, g_stScanCodeListMem.cID);
							
   #ifdef DEBUG_APP_TASK		
								printf("\r\n");
								printf(" cID:%d ", g_stScanCodeListMem.cID);       
								printf(" pName:%s", g_stScanCodeListMem.pName);				 
								printf(" datetime:%s ", g_stScanCodeListMem.datetime);
								printf(" cStatus:%d ", g_stScanCodeListMem.cStatus);
								printf("\r\n");
		#endif					
								ret = TRUE;
						}
				}
        
				//��ΪԤ��
				
        cJSON_Delete(root);
    }
 
    return ret;
}

u8 ParseVersionJson(const char* pszJsonText)
{
    cJSON	*root, *result;
 
    root= cJSON_Parse(pszJsonText);
 
    if (root)
    { 
        result = cJSON_GetObjectItem(root, "result");
						
        if (result)
        { 
						g_uVersion =  cJSON_GetObjectItem(result, "version")->valueint;
											
#ifdef DEBUG_APP_TASK		
						printf("\r\n");    
						printf(" version:%d", g_uVersion);				 
						printf("\r\n");
#endif					
        }
        cJSON_Delete(root);
    }
 
    return TRUE;
}
