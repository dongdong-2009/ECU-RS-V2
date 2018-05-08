#include "ECUClient.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"
#include "debug.h"
#include "serverfile.h"
#include "socket.h"
#include "stdlib.h"
#include "string.h"
#include "threadlist.h"
#include "cJSON.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
clientResponse_t clientResponseRet;		//��EMAͨѶ�����ݽ����ṹ��

//ClientͨѶ�õ����ַ�������Ϊ�ṹ��
int clientResponse_toStruct(const char *str,clientResponse_t *clientResponse)
{
    char Flagstr[2] = {'\0'};
    char DateTime_NumStr[3] = {'\0'};
    unsigned char index = 0;

    clientResponse->Flag = 0;
    clientResponse->DateTime_Num = 0;
    if(str[0] != '1')
        return -1;
    Flagstr[0] = str[0];
    Flagstr[1] = '\0';
    memcpy(DateTime_NumStr,&str[1],2);
    DateTime_NumStr[2] = '\0';
    clientResponse->Flag = atoi(Flagstr);
    clientResponse->DateTime_Num = atoi(DateTime_NumStr);
    for(index = 0;index < clientResponse->DateTime_Num;index++)
    {
        memcpy(clientResponse->DateTime[index],&str[3+14*index],14);
    }
    return 0;



}

//ClientͨѶ�õ���JSON����Ϊ�ṹ��
int clientResponseJSON_toStruct(const char *json_str,clientResponse_t *clientResponse)
{
    cJSON *root,*arrayItem,*object;
    int size = 0,i = 0;
    clientResponse->Flag = 0;
    clientResponse->DateTime_Num = 0;

    root=cJSON_Parse(json_str);
    if (!root)
    {
        return -1;
    }
    else
    {
        cJSON *item=cJSON_GetObjectItem(root,"F");
        if(item!=NULL)
        {
            clientResponse->Flag = atoi(item->valuestring);
        }
        item=cJSON_GetObjectItem(root,"DN");
        if(item!=NULL)
        {
            clientResponse->DateTime_Num = atoi(item->valuestring);
        }

        arrayItem=cJSON_GetObjectItem(root,"DT");
        if(arrayItem!=NULL)
        {
            size=cJSON_GetArraySize(arrayItem);
            for(i=0;i<size;i++)
            {
                if(i >=99) break;
                object=cJSON_GetArrayItem(arrayItem,i);
                memcpy(clientResponse->DateTime[i],object->valuestring,14);
            }
        }
        cJSON_Delete(root);
        return 0;
    }
}



//����ͷ��Ϣ��EMA,��ȡ�Ѿ�����EMA�ļ�¼ʱ��
int preprocess(void)			
{
    int sendbytes = 0,readbytes = 0;
    char *readbuff = NULL;
    char sendbuff[50] = {'\0'};

    if(0 == detection_resendflag2())		//	����Ƿ���resendflag='2'�ļ�¼
        return 0;
    readbuff = malloc((50+99*18));
    memset(readbuff,0x00,(50+99*18));
    readbytes = 50+99*18;
    sprintf(sendbuff,"{\"EID\":\"%12s\",}\n",ecu.ECUID12);
	
    print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

    //���͵�������
    sendbytes = serverCommunication_Client(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
    if(-1 == sendbytes)
    {
        free(readbuff);
        readbuff = NULL;
        return -1;
    }
    if(readbytes >3)
    {
    	 if(readbuff[0] == '{')
        {//�ж�ΪJSON����
            clientResponseJSON_toStruct(readbuff,&clientResponseRet);
        }else
        {//�ж�Ϊ�ַ�������
            clientResponse_toStruct(readbuff,&clientResponseRet);
        }
        clear_send_flag(clientResponseRet);

    }else
    {
        free(readbuff);
        readbuff = NULL;
        return -1;
    }
    free(readbuff);
    readbuff = NULL;
    return 0;
}

int send_record(char *sendbuff, char *send_date_time)			//�������ݵ�EMA  ע���ڴ洢��ʱ���βδ���'\n'  �ڷ���ʱ��ʱ��ǵ����
{
    int sendbytes=0,readbytes = 50+99*18;
    char *readbuff = NULL;
    readbuff = malloc((50+99*18));
    memset(readbuff,'\0',(50+99*18));
    sendbytes = serverCommunication_Client(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
    if(-1 == sendbytes)
    {
        free(readbuff);
        readbuff = NULL;
        return -1;
    }

    if(readbytes < 3)
    {
        free(readbuff);
        readbuff = NULL;
        return -1;
    }
    else
    {
        print2msg(ECU_DBG_CLIENT,"readbuff",readbuff);
        //�ж���JSON���ݻ���String����
        if(readbuff[0] == '{')
        {//�ж�ΪJSON����
            clientResponseJSON_toStruct(readbuff,&clientResponseRet);
        }else
        {//�ж�Ϊ�ַ�������
            clientResponse_toStruct(readbuff,&clientResponseRet);
        }

        if(1 == clientResponseRet.Flag)
            update_send_flag(send_date_time);
        clear_send_flag(clientResponseRet);
        free(readbuff);
        readbuff = NULL;
        return 0;
    }
}


int resend_record(char *data)
{
    char time[15] = {'\0'};
    int flag,res;

    memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
    //��/home/record/data/Ŀ¼�²�ѯresendflagΪ2�ļ�¼
    while(search_readflag(data,time,&flag,'2'))		//	��ȡһ��resendflagΪ1������
    {
        //if(1 == flag)		// ��������Ҫ�ϴ�������
        //		data[78] = '1';
        printmsg(ECU_DBG_CLIENT,data);
        res = send_record(data, time);
        if(-1 == res)
            break;
    }

    return 0;
}

//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUClient_thread_entry(void* parameter)
{
    int ClientThistime=0, ClientDurabletime=65535, ClientReportinterval=300;
    char broadcast_hour_minute[3]={'\0'};
    char broadcast_time[16];
    char *data = NULL;
    int res,flag;
    char time[15] = {'\0'};
    rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_CLIENT);
    data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON);
    memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON);

    while(1)
    {
        if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
            printmsg(ECU_DBG_CLIENT,"Client Start");
            ClientDurabletime = acquire_time();
            ClientThistime = acquire_time();
            //1�����2����Ҫ���־λ
            if(1 == get_hour())
            {
                preprocess();
                resend_record(data);
                delete_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
            }
            get_time(broadcast_time, broadcast_hour_minute);
            print2msg(ECU_DBG_CLIENT,"time",broadcast_time);

            memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON);
            while(search_readflag(data,time,&flag,'1'))		//	��ȡһ��resendflagΪ1������
            {
                if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval))
                {
                    break;
                }
                //if(1 == flag)		// ��������Ҫ�ϴ�������
                //data[88] = '1';
                //printmsg(ECU_DBG_CLIENT,data);
                res = send_record( data, time);
                if(-1 == res)
                    break;
                ClientThistime = acquire_time();
                memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON));
                memset(time,0,15);
            }


            delete_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼

            printmsg(ECU_DBG_CLIENT,"Client End");

        }

        rt_thread_delay(RT_TICK_PER_SECOND);
        ClientDurabletime = acquire_time();
    }
}


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(preprocess , preprocess().)
#endif
