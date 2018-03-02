/*****************************************************************************/
/* File      : timer.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-01 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
/*****************************************************************************/
/* 定时器2  用于10分钟计时																								   */
/* 定时器3  用于串口数据接收超时的使用 																		   */
/*****************************************************************************/

#include "timer.h"
#include "led.h"
#include "usart5.h"
#include "SEGGER_RTT.h"
#include "watchdog.h"


static signed char usart_flag = 0;
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	TIM_Cmd(TIM3, ENABLE);  //使能TIMx		
	usart_flag = 0;
}

void TIM3_Int_Deinit(void)
{
	usart_flag = 0;
	TIM_Cmd(TIM3, DISABLE);
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
			usart_flag++;
			if( usart_flag > 1)
			{
				SEGGER_RTT_printf(0, "TIM3_IRQHandler\n");
				TIM3_Int_Deinit();
			}
		}
}


signed char COMM_Timeout_Event = 0;
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	
	//定时器TIM2初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM2中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx		
	COMM_Timeout_Event = 0;
}

void TIM2_Int_Deinit(void)
{
	COMM_Timeout_Event = 0;
	TIM_Cmd(TIM2, DISABLE);
}

void TIM2_Refresh(void)
{
	COMM_Timeout_Event = 0;
}

int timeout_WIFI_RST_Time =0;
//定时器2中断服务程序
void TIM2_IRQHandler(void)   //TIM2中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 
			timeout_WIFI_RST_Time++;
			//产生超时时间
			COMM_Timeout_Event = 1;

			if(timeout_WIFI_RST_Time > 64800)
			{
				timeout_WIFI_RST_Time = 0;
			}

		}
}
