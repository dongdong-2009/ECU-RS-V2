/*****************************************************************************/
/*  File      : mycommand.c                                                  */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-13 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "rthw.h"
#include "threadlist.h"
#include <rtthread.h>


//��ʱ��
rt_timer_t readtimer;

//��ʱ����ʱ����
static void reboottimeout(void* parameter)
{
	reboot();
}

void reboot_timer(int timeout)			//zigbee�������ݼ�� ����0 ��ʾ����û������  ����1��ʾ����������
{
	readtimer = rt_timer_create("read", /* ��ʱ������Ϊ read */
					reboottimeout, /* ��ʱʱ�ص��Ĵ����� */
					RT_NULL, /* ��ʱ��������ڲ��� */
					timeout*RT_TICK_PER_SECOND, /* ��ʱʱ�䳤��,��OS TickΪ��λ*/
					 RT_TIMER_FLAG_ONE_SHOT); /* �����ڶ�ʱ�� */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
}

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int mysystem(const char *command)
{
	int res;

	print2msg(ECU_DBG_CONTROL_CLIENT,"Execute:",(char*) command);
	//��Ҫ�������ڴ����
	if(!memcmp(command,"reboot",6))
	{
		res = 0;
		//ͨ����ʱ����λ����
		reboot_timer(10);
	}else if(!memcmp(command,"restart UPDATE",14))
	{
		restartThread(TYPE_UPDATE);
		res = 0;
	}else if(!memcmp(command,"ftpput",6))
	{
		//�ϴ�����
	}else if(!memcmp(command,"ftpget",6))
	{
		//��������
	}
		
	printdecmsg(ECU_DBG_CONTROL_CLIENT,"res",res);
	if(-1 == res){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: system error.");
		return CMD_ERROR;
	}
	else{

		if(0 == res){
			printmsg(ECU_DBG_CONTROL_CLIENT,"Execute successfully.");
		}
		else{
			printdecmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: shell failed", res);
			return CMD_ERROR;
		}

	}
	return SUCCESS;
}
