#include "ECUControl.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"
#include "debug.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


//该线程主要用于数据上传以及远程控制
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//远程控制 15分钟上报	
			printmsg(ECU_DBG_CONTROL_CLIENT,"ECUControl_thread_entry     Control DATA  Start-------------------------");

			ControlThistime = acquire_time();
			
			printmsg(ECU_DBG_COLLECT,"ECUControl_thread_entry     Control DATA  End-------------------------");
		}
		
		//上报告警标志
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();			

	}
}
