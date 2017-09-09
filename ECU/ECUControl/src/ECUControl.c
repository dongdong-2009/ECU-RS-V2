#include "ECUControl.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"
#include "debug.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//Զ�̿��� 15�����ϱ�	
			printmsg(ECU_DBG_CONTROL_CLIENT,"ECUControl_thread_entry     Control DATA  Start-------------------------");

			ControlThistime = acquire_time();
			
			printmsg(ECU_DBG_COLLECT,"ECUControl_thread_entry     Control DATA  End-------------------------");
		}
		
		//�ϱ��澯��־
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();			

	}
}
