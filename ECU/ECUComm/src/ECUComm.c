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
extern unsigned char rateOfProgress;
int init_all(inverter_info *inverter)
{
	rateOfProgress = 0;
	openzigbee();
	zigbee_reset();
	zb_test_communication();
	init_ecu();
	init_inverter(inverter);
	rateOfProgress = 100;
	init_tmpdb(inverter);
	return 0;
}

void ECUComm_thread_entry(void* parameter)
{
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_COMM);
	init_all(inverterInfo); //初始化所有逆变器
	while(1)
	{
		//判断是否有433模块心跳超时事件
		if(COMM_Timeout_Event == 1)
		{
			//SEGGER_RTT_printf(0,"COMM_Timeout_Event start\n");
			process_HeartBeatEvent();			
			
			COMM_Timeout_Event = 0;
			//SEGGER_RTT_printf(0,"COMM_Timeout_Event end\n");
		}
		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}
}
