#include "TrinaSolar.h"
#include "socket.h"

Socket_Cfg TrinaSolar_arg;
int TrinaSolarSocketFD = -1;		//���SOCKET�ļ�������

void TrinaSolar_thread_entry(void* parameter)	//����߳�
{
	int i = 0;
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_TRINASOLAR);
	


}
