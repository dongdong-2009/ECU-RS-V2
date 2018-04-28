#ifndef __ECU_CLIENT_H__
#define __ECU_CLIENT_H__

#define DATETIME_NUM     99
#define DATETIME_LEN      14


typedef struct
{
    unsigned char Flag;
    unsigned char DateTime_Num;
    char DateTime[DATETIME_NUM][DATETIME_LEN];
}clientResponse_t;

void ECUClient_thread_entry(void* parameter);
#endif /*__ECU_CLIENT_H__*/
