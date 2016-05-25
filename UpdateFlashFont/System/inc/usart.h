#ifndef __USART_H
#define __USART_H
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stdio.h"


#define CMD_BUF_LENGTH 50					  //�����ĳ���
#define RECV_BUF_LENGTH 10240

#define HEADER0  0xFF
#define HEADER1  0xF5
#define END0     0x0D
#define END1     0x0A

typedef struct
{
  uint8_t Header[2];
	uint8_t length[2];//��λ��ǰ
//	uint8_t Type;
//	uint8_t Command;
  uint8_t RecvBuf[10*1024]; //������type command crc
//	uint8_t crc[4];//��λ��ǰ
  uint8_t End[2];
} USART_RecvMsgDef;

void UART2_SendLR(void);
void UART2_SendString(char* s);
void All_USART_Init(void);

//����2���ͻس�����
#define UART2_SendLR() UART2_SendString("\r\n")

void CLR_Buf2(void);

#endif
