#ifndef __TRINASOLAR_H__
#define __TRINASOLAR_H__
int TrinaFunction(void);
void TrinaSolar_heartbeat_thread_entry(void* parameter);	//��������߳�
void TrinaSolar_recv_thread_entry(void* parameter);	//��Ͻ����߳�
void TrinaSolar_thread_entry(void* parameter);
#endif /*__TRINASOLAR_H__*/
