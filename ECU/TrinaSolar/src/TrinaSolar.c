#include "TrinaSolar.h"
#include "socket.h"

Socket_Cfg TrinaSolar_arg;
int TrinaSolarSocketFD = -1;		//天合SOCKET文件描述符

void TrinaSolar_thread_entry(void* parameter)	//天合线程
{
	int i = 0;
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_TRINASOLAR);
	


}
