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
#include "RFM300H.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "file.h"

void ECUComm_thread_entry(void* parameter)
{
	while(1)
	{
		//判断是否有433模块心跳超时事件
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
