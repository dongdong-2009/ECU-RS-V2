#include "ECUComm.h"
#include "event.h"
#include "led.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "Flash_24L512.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "zigbee.h"
#include "serverfile.h"
#include "threadlist.h"
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

int init_all(inverter_info *inverter)
{
	openzigbee();
	zb_test_communication();
	init_ecu();
	init_inverter(inverter);
	init_tmpdb(inverter);
	return 0;
}

void ECUComm_thread_entry(void* parameter)
{
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_COMM);
	init_all(inverterInfo); //��ʼ�����������
	while(1)
	{
		//�ж��Ƿ���433ģ��������ʱ�¼�
		if(COMM_Timeout_Event == 1)
		{
			//SEGGER_RTT_printf(0,"COMM_Timeout_Event start\n");
			process_HeartBeatEvent();			
			kickwatchdog();
			COMM_Timeout_Event = 0;
			//SEGGER_RTT_printf(0,"COMM_Timeout_Event end\n");
		}
		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}
}
