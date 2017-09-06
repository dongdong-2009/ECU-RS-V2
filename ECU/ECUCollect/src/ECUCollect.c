#include "ECUCollect.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];



//���߳���Ҫ����������ݵĲɼ�����
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
			//5���Ӳɼ���صķ���������
			ClientThistime = acquire_time();
			//�ɼ�һ������
			curinverter = inverterInfo;
			for(i = 0;i< ecu.validNum; i++)
			{
				
				
				curinverter++;
			}
			//�ϱ�����
			
		}
		
		
		
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//�ɼ��������Զ�̿�������
			
		}

		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
		ControlDurabletime = acquire_time();			
	}
}
