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
clientResponse_t clientResponseRet;		//和EMA通讯后数据解析结构体

//Client通讯得到的字符串解析为结构体
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

//Client通讯得到的JSON解析为结构体
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



//发送头信息到EMA,读取已经存在EMA的记录时间
int preprocess(void)			
{
    int sendbytes = 0,readbytes = 0;
    char *readbuff = NULL;
    char sendbuff[50] = {'\0'};

    if(0 == detection_resendflag2())		//	检测是否有resendflag='2'的记录
        return 0;
    readbuff = malloc((50+99*18));
    memset(readbuff,0x00,(50+99*18));
    readbytes = 50+99*18;
    sprintf(sendbuff,"{\"EID\":\"%12s\",}\n",ecu.ECUID12);
	
    print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

    //发送到服务器
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
        {//判断为JSON数据
            clientResponseJSON_toStruct(readbuff,&clientResponseRet);
        }else
        {//判断为字符串数据
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

int send_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
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
        //判断是JSON数据还是String数据
        if(readbuff[0] == '{')
        {//判断为JSON数据
            clientResponseJSON_toStruct(readbuff,&clientResponseRet);
        }else
        {//判断为字符串数据
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
    //在/home/record/data/目录下查询resendflag为2的记录
    while(search_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
    {
        //if(1 == flag)		// 还存在需要上传的数据
        //		data[78] = '1';
        printmsg(ECU_DBG_CLIENT,data);
        res = send_record(data, time);
        if(-1 == res)
            break;
    }

    return 0;
}

//该线程主要用于数据上传以及远程控制
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
            //1点或者2点需要清标志位
            if(1 == get_hour())
            {
                preprocess();
                resend_record(data);
                delete_file_resendflag0();		//清空数据resend标志全部为0的目录
            }
            get_time(broadcast_time, broadcast_hour_minute);
            print2msg(ECU_DBG_CLIENT,"time",broadcast_time);

            memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON);
            while(search_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
            {
                if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval))
                {
                    break;
                }
                //if(1 == flag)		// 还存在需要上传的数据
                //data[88] = '1';
                //printmsg(ECU_DBG_CLIENT,data);
                res = send_record( data, time);
                if(-1 == res)
                    break;
                ClientThistime = acquire_time();
                memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER+CLIENT_RECORD_JSON));
                memset(time,0,15);
            }


            delete_file_resendflag0();		//清空数据resend标志全部为0的目录

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
