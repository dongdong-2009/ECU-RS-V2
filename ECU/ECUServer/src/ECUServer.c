#include "ECUServer.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"

//���ݲɼ�
void dataCollection(void)
{
	
}

//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUServer_thread_entry(void* parameter)
{
	int ClientThistime=0, ClientDurabletime=65535, ClientReportinterval=300;
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
			//��ʱ�ϱ����� 5�����ϱ�
			ClientThistime = acquire_time();
			//�ɼ�һ������
			
			//�ϱ�����
			
		}
		
		
		
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	//Զ�̿��� 15�����ϱ�
			
		}
		
		//�ϱ��澯��־
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
		ControlDurabletime = acquire_time();			

		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}
}
