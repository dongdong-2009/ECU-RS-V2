#ifndef __ECU_COLLECT_H__
#define __ECU_COLLECT_H__

typedef enum
{ 
    EN_ECUHEART_DISABLE 	= 0,	//接收Socket数据头
    EN_ECUHEART_ENABLE    = 1,	//接收手机Socket ID
} eEcuHeartStatus;// receive state machin


void ECUCollect_thread_entry(void* parameter);
#endif /*__ECU_COLLECT_H__*/
