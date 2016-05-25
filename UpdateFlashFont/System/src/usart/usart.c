#include "usart.h"
#include "cjsonutil.h"

//extern u8 IsClearGlobalParam;
//extern u8 IsConnectServer;

/*************  ���ر�������	**************/

char Uart2Buf[CMD_BUF_LENGTH]; //����2���ջ���
char *p1=Uart2Buf;

u32 g_uRecvBufIndex = 0;
u8  g_cStartRecvMsg = 0;
char RecvBuf[RECV_BUF_LENGTH];

//u16 RealRecvBufLength = 0;

int fputc(int ch, FILE *f)
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (u8) ch);

  /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {
  }

  return ch;
}

void UART2_SendString(char* s)
{
	while(*s != '\0')//����ַ���������
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
		USART_SendData(USART2 ,*s++);//���͵�ǰ�ַ�
	}
}

/*************************************************************************
 * Function Name: Serial_Init
 * Description: Init USARTs
 *************************************************************************/
static void Serial_First_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
 // NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
  //RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, DISABLE);

  /* Configure USART1 Tx (PA.09) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART1 Rx (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
		
/* USART1 configuration ------------------------------------------------------*/
  // USART1 configured as follow:
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		//Usart1 NVIC ����
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//

//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
//	NVIC_Init(&NVIC_InitStructure);	//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���USART1
	
  /* Configure the USART1 */
  USART_Init(USART1, &USART_InitStructure);

	/*����1ֻ������Ϣ�����*/
  /* Enable the USART Receive interrupt: this interrupt is generated when the 
     USART1 receive data register is not empty */
 // USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);
}

static void Serial_Second_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	 
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PORTAʱ��
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //ʹ��USART2
 	  USART_DeInit(USART2);  //��λ����2
	  //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3     
	
	  //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	  NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  ������
        - Word Length = 8 Bits  ���ݳ���
        - One Stop Bit          ֹͣλ
        - No parity             У�鷽ʽ
        - Hardware flow control disabled (RTS and CTS signals) Ӳ��������
        - Receive and transmit enabled                         ʹ�ܷ��ͺͽ���
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 	
}

void All_USART_Init(void)
{
		Serial_First_Init();
		Serial_Second_Init();
}

//�������ͷ�жϼ�У��
//header   ֡ͷ 		0xFF 0xF5
//type     ��������		1�ֽ�
/*
0x00 ��ʾΪ״ָ̬���ȡ������״̬
0x01 ��ʾ����ָ����ơ����õ�
0x02 ��ʾACK��command��parameterΪ0
*/
//command  ����       1�ֽ�
//length   ����  			4�ֽ�                   parameter����  
//parameter ����      1-n
//crc     У����		 4�ֽ�

//end       ������   0x0D 0x0A                 ����ش�����?
void USART1_IRQHandler(void)                	//����1�жϷ������
{
  
}

//����2�ж�
void USART2_IRQHandler(void)  
{  		
		u8 Res;
	
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�
    {
        Res = USART_ReceiveData(USART2); //(USART1->DR);	//��ȡ���յ�������
			
				if(g_cStartRecvMsg == 1)
				{
						RecvBuf[g_uRecvBufIndex++] = Res;
				}
				else
				{
						*p1 = Res;  	  //�����յ����ַ����浽������			
						p1++;                			//����ָ������ƶ�
						if(p1>&Uart2Buf[CMD_BUF_LENGTH])       		//���������,������ָ��ָ�򻺴���׵�ַ
						{
							p1=Uart2Buf;
						} 
			  }
		}
		
		//������ݴ���
		if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
    {
        Res = USART_ReceiveData(USART2); //(USART1->DR);	//��ȡ���յ�������
			
				if(g_cStartRecvMsg == 1)
				{
						RecvBuf[g_uRecvBufIndex++] = Res;
				}
				else
				{
						*p1 = Res;  	  //�����յ����ַ����浽������			
						p1++;                			//����ָ������ƶ�
						if(p1>&Uart2Buf[CMD_BUF_LENGTH])       		//���������,������ָ��ָ�򻺴���׵�ַ
						{
							p1=Uart2Buf;
						} 
			  }
				
        USART_ClearFlag(USART2,USART_FLAG_ORE);
    }
}

/*******************************************************************************
* ������ : CLR_Buf2
* ����   : �������2��������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<CMD_BUF_LENGTH;k++)      //��������������
	{
		Uart2Buf[k] = 0x00;
	}
	
	p1=Uart2Buf;  
}
