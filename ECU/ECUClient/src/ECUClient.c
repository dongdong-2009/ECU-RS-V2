#include "ECUClient.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUClient_thread_entry(void* parameter)
{
	int ClientThistime=0, ClientDurabletime=65535, ClientReportinterval=300;

	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
			//��ʱ�ϱ����� 5�����ϱ�
			ClientThistime = acquire_time();
			//�ɼ�һ������
			
			//�ϱ�����
			
		}
		
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
	}
}
