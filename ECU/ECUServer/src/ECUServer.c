#include "ECUServer.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"

//数据采集
void dataCollection(void)
{
	
}

//该线程主要用于数据上传以及远程控制
void ECUServer_thread_entry(void* parameter)
{
	int ClientThistime=0, ClientDurabletime=65535, ClientReportinterval=300;
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
			//定时上报数据 5分钟上报
			ClientThistime = acquire_time();
			//采集一轮数据
			
			//上报数据
			
		}
		
		
		
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	//远程控制 15分钟上报
			
		}
		
		//上报告警标志
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
		ControlDurabletime = acquire_time();			

		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}
}
