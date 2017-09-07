#include "ECUControl.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


//该线程主要用于数据上传以及远程控制
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	//远程控制 15分钟上报
			printf("ECUCollect_thread_entry\n");
		}
		
		//上报告警标志
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();			

	}
}
