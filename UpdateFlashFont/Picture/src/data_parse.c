#include "data_parse.h"
#include "usart.h"
#include "jpg_create.h"
#include "piclib.h"

//ע�����ݵĳ���ҪΪ16λ
static u32 CRC321(unsigned char *ptr, u16 len)
{
	unsigned char i;
	unsigned long  crc=0xffffffff;
	while(len--!=0) 
	{
		crc ^= *ptr++;
		for(i=0; i<8;i++) 
		{
			if(crc&1)
		 	{
				crc >>= 1;
				crc ^= 0xEDB88320;
			}
			else 
				crc >>= 1;
		}
	}
	return(crc);
}

void DataPrase(void)
{
#if 0
		u8 type = 0;
		u8 command = 0;
		u32 crc0 = 0;
		u32 crc1 = 0;
		u8 i = 0;

		type = USART_RecvMsg.RecvBuf[0];
		command = USART_RecvMsg.RecvBuf[1];
		//����ӵڶ�λ���ݿ�ʼ��CRC��
		crc0 = CRC321(&USART_RecvMsg.RecvBuf[2], RealRecvBufLength-6);
	
		//ȡ������λ�·���CRC��
		crc1 = (USART_RecvMsg.RecvBuf[RealRecvBufLength-4] << 24) + (USART_RecvMsg.RecvBuf[RealRecvBufLength-3] << 16)
					+ (USART_RecvMsg.RecvBuf[RealRecvBufLength-2] << 8) + USART_RecvMsg.RecvBuf[RealRecvBufLength-1];

		if(crc0 == crc1)
		{
				printf("CRC OK!\n");
			
				if(CreateJPGFile() == FR_OK)
				{
						ai_load_picfile("0:/test.jpg",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ  
					
						printf("CreateJPGFile OK!\n");
				}
		}
#endif
}
