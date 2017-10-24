/*****************************************************************************/
/* File      : key.c                                                         */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "key.h"
#include <stm32f10x.h>
#include "SEGGER_RTT.h"
#include "rthw.h"
#include "stdio.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

#define KEY_RCC              	RCC_APB2Periph_GPIOB
#define KEY_GPIO              GPIOB
#define KEY_PIN               (GPIO_Pin_9)

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//按键IO初始化
void KEY_Init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(KEY_RCC,ENABLE);//使能portB的时钟
	
	GPIO_InitStructure.GPIO_Pin  = KEY_PIN;//PB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(KEY_GPIO, &GPIO_InitStructure);//初始化PB9
	
}

//中断初始化
void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  KEY_Init();	 //按键端口初始化

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使用复用时钟功能
	
	//GPIOB.9 中断线以及中断初始化配置   下降沿触发
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource9);
 	EXTI_InitStructure.EXTI_Line=EXTI_Line9;	//KEY_RESET
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
 	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
 	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
 	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

 	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键KEY_RESET所在的外部中断通道
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//抢占优先级2
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
 	NVIC_Init(&NVIC_InitStructure);  

}
signed char KEY_FormatWIFI_Event = 0;
//外部中断9_5服务程序
void EXTI9_5_IRQHandler(void)
{
	rt_hw_ms_delay(20);
	if(KEY_Reset==1)	 	 
	{
		printf("KEY_FormatWIFI_Event\n");
		KEY_FormatWIFI_Event = 1;

	}
	EXTI_ClearITPendingBit(EXTI_Line9); //清除LINE9上的中断标志位 
}
