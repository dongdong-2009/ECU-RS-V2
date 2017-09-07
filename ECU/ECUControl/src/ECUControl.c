#include "ECUControl.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	//Զ�̿��� 15�����ϱ�
			printf("ECUCollect_thread_entry\n");
		}
		
		//�ϱ��澯��־
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();			

	}
}
