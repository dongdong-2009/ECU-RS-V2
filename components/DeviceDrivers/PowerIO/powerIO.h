#ifndef __POWERIO_H__
#define __POWERIO_H__

#include <rtthread.h>

void rt_hw_powerIO_init(void);	//PowerIO��ʼ��
void rt_hw_powerIO_on(void);		//PowerIO�ߵ�ƽ
void rt_hw_powerIO_off(void);		//PowerIO�͵�ƽ

void rt_hw_ETHIO_init(void);	//����״̬IO��ʼ��
int rt_hw_ETHIO_status(void);	//��ǰ����IO״̬
#endif
