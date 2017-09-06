#include "ECUCollect.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];



//该线程主要用于相关数据的采集工作
void ECUCollect_thread_entry(void* parameter)
{
	int i = 0;
	int ClientThistime=0, ClientDurabletime=65535, ClientReportinterval=300;
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	inverter_info *curinverter = inverterInfo;
	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval))
		{
			//5分钟采集相关的发电量数据
			ClientThistime = acquire_time();
			//采集一轮数据
			curinverter = inverterInfo;
			for(i = 0;i< ecu.validNum; i++)
			{
				
				
				curinverter++;
			}
			//上报数据
			
		}
		
		
		
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//采集心跳相关远程控制数据
			
		}

		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
		ControlDurabletime = acquire_time();			
	}
}
