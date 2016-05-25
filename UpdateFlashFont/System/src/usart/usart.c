#include "usart.h"
#include "cjsonutil.h"

//extern u8 IsClearGlobalParam;
//extern u8 IsConnectServer;

/*************  本地变量声明	**************/

char Uart2Buf[CMD_BUF_LENGTH]; //串口2接收缓存
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
	while(*s != '\0')//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
		USART_SendData(USART2 ,*s++);//发送当前字符
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

		//Usart1 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//

//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1
	
  /* Configure the USART1 */
  USART_Init(USART1, &USART_InitStructure);

	/*串口1只用于信息的输出*/
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
	 
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能PORTA时钟
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //使能USART2
 	  USART_DeInit(USART2);  //复位串口2
	  //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3     
	
	  //Usart1 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	  NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  波特率
        - Word Length = 8 Bits  数据长度
        - One Stop Bit          停止位
        - No parity             校验方式
        - Hardware flow control disabled (RTS and CTS signals) 硬件控制流
        - Receive and transmit enabled                         使能发送和接收
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART2, ENABLE);                    //使能串口 	
}

void All_USART_Init(void)
{
		Serial_First_Init();
		Serial_Second_Init();
}

//添加数据头判断及校验
//header   帧头 		0xFF 0xF5
//type     命令类型		1字节
/*
0x00 表示为状态指令，获取、返回状态
0x01 表示命令指令、控制、设置等
0x02 表示ACK，command、parameter为0
*/
//command  命令       1字节
//length   长度  			4字节                   parameter长度  
//parameter 数据      1-n
//crc     校验码		 4字节

//end       结束符   0x0D 0x0A                 添加重传机制?
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
  
}

//串口2中断
void USART2_IRQHandler(void)  
{  		
		u8 Res;
	
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断
    {
        Res = USART_ReceiveData(USART2); //(USART1->DR);	//读取接收到的数据
			
				if(g_cStartRecvMsg == 1)
				{
						RecvBuf[g_uRecvBufIndex++] = Res;
				}
				else
				{
						*p1 = Res;  	  //将接收到的字符串存到缓存中			
						p1++;                			//缓存指针向后移动
						if(p1>&Uart2Buf[CMD_BUF_LENGTH])       		//如果缓存满,将缓存指针指向缓存的首地址
						{
							p1=Uart2Buf;
						} 
			  }
		}
		
		//溢出数据处理
		if(USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET)
    {
        Res = USART_ReceiveData(USART2); //(USART1->DR);	//读取接收到的数据
			
				if(g_cStartRecvMsg == 1)
				{
						RecvBuf[g_uRecvBufIndex++] = Res;
				}
				else
				{
						*p1 = Res;  	  //将接收到的字符串存到缓存中			
						p1++;                			//缓存指针向后移动
						if(p1>&Uart2Buf[CMD_BUF_LENGTH])       		//如果缓存满,将缓存指针指向缓存的首地址
						{
							p1=Uart2Buf;
						} 
			  }
				
        USART_ClearFlag(USART2,USART_FLAG_ORE);
    }
}

/*******************************************************************************
* 函数名 : CLR_Buf2
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<CMD_BUF_LENGTH;k++)      //将缓存内容清零
	{
		Uart2Buf[k] = 0x00;
	}
	
	p1=Uart2Buf;  
}
