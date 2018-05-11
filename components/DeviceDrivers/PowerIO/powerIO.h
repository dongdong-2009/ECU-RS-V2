#ifndef __POWERIO_H__
#define __POWERIO_H__

#include <rtthread.h>

void rt_hw_powerIO_init(void);	//PowerIO初始化
void rt_hw_powerIO_on(void);		//PowerIO高电平
void rt_hw_powerIO_off(void);		//PowerIO低电平

void rt_hw_ETHIO_init(void);	//网络状态IO初始化
int rt_hw_ETHIO_status(void);	//当前网络IO状态
#endif
