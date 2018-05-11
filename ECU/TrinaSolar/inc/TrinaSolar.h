#ifndef __TRINASOLAR_H__
#define __TRINASOLAR_H__
int TrinaFunction(void);
void TrinaSolar_heartbeat_thread_entry(void* parameter);	//天合心跳线程
void TrinaSolar_recv_thread_entry(void* parameter);	//天合接受线程
void TrinaSolar_thread_entry(void* parameter);
#endif /*__TRINASOLAR_H__*/
