#include "ECUServer.h"
#include "rtthread.h"
void ECUServer_thread_entry(void* parameter)
{
	while(1)
	{
		//��ʱ�ϱ�����
		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}
}
