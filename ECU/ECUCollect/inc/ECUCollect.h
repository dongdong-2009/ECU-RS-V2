#ifndef __ECU_COLLECT_H__
#define __ECU_COLLECT_H__

typedef enum
{ 
    EN_ECUHEART_DISABLE 	= 0,	//����Socket����ͷ
    EN_ECUHEART_ENABLE    = 1,	//�����ֻ�Socket ID
} eEcuHeartStatus;// receive state machin


void ECUCollect_thread_entry(void* parameter);
#endif /*__ECU_COLLECT_H__*/
