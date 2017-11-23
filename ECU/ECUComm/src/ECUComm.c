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
#include "zigbee.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern unsigned char rateOfProgress;
extern unsigned char ECUCommThreadFlag;


void ECUComm_thread_entry(void* parameter)
{
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_COMM);
	while(1)
	{
		//判断是否有433模块心跳超时事件
		if((ECUCommThreadFlag == 1) && (ecu.validNum > 0))
		{
			if(COMM_Timeout_Event == 1)
			{
				if(ecu.curHeartSequence >= ecu.validNum)
				{
					ecu.curHeartSequence = 0;
				}
				zb_sendHeart(inverterInfo[ecu.curHeartSequence].uid);	
				ecu.curHeartSequence++;
				COMM_Timeout_Event = 0;
			}
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND/50);
	}
}
