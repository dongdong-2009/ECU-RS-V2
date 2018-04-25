/*****************************************************************************/
/* File      : 485usart.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-01-17 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "usart485.h"	
#include "SEGGER_RTT.h"
#include "SEGGER_RTT.h"
#include "string.h"
#include "stm32f10x.h"
#include "rthw.h"
#include "stdio.h"
#include "rtthread.h"
#include "stdlib.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define RX485_RCC                    RCC_APB2Periph_GPIOA
#define RX485_GPIO                   GPIOA
#define RX485_PIN                    (GPIO_Pin_3)

#define TX485_RCC                    RCC_APB2Periph_GPIOB
#define TX485_GPIO                   GPIOB
#define TX485_PIN                    (GPIO_Pin_10)

//bound:波特率
void usart485_init(unsigned int bound){
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3, ENABLE);	//使能串口时钟USART2 USART3
	RCC_APB2PeriphClockCmd(RX485_RCC|TX485_RCC|RCC_APB2Periph_AFIO, ENABLE);	//使能485RX/485TX时钟

	//485TX
	GPIO_InitStructure.GPIO_Pin = TX485_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(TX485_GPIO, &GPIO_InitStructure); 
   
    //485RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PD2
	
   //USART TX 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口

	   //USART RX 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口
	
   //UART5 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART2, ENABLE);                    //使能串口 
	USART_Cmd(USART3, ENABLE);                    //使能串口

}

unsigned char USART2_RX_BUF[USART2_REC_LEN];     			
unsigned short USART2Cur = 0;	


void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		SEGGER_RTT_printf(0, "[%d] : %x %c\n",USART2Cur,USART2_RX_BUF[USART2Cur],USART2_RX_BUF[USART2Cur]);
		USART2_RX_BUF[USART2Cur] = USART_ReceiveData(USART2);
		USART2Cur +=1;
		if(USART2Cur >=USART2_REC_LEN)
		{
			USART2Cur = 0;
		}
	}
} 
void clear_485(void)
{
	USART2Cur = 0;
}

int Send485Data(char *data, int num)
{
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 
    USART_SendData(USART3,(uint16_t)ch);
	}
	return index;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(usart485_init , usart485_init.)
FINSH_FUNCTION_EXPORT(Send485Data , Send485Data.)

#endif
