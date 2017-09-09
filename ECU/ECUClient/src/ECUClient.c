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

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

//发送头信息到EMA,读取已经存在EMA的记录时间
int preprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char sendbuff[50] = {'\0'};

	if(0 == detection_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((4+99*14));
	memset(readbuff,0x00,(4+99*14));
	readbytes = 4+99*14;
	strcpy(sendbuff, "APS13AAA22");
	memcpy(&sendbuff[10],ecu.ECUID12,12);
	strcat(sendbuff, "\n");
	print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Client(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		return -1;
	}
	if(readbytes >3)
	{
		clear_send_flag(readbuff);
		
	}else
	{
		return -1;
	}
	return 0;
}

int send_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = 4+99*14;
	char *readbuff = NULL;
	readbuff = malloc((4+99*14));
	memset(readbuff,'\0',(4+99*14));
	
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
		//print2msg(ECU_DBG_CLIENT,"readbuff",readbuff);
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		if(1 == flag)		// 还存在需要上传的数据
				data[78] = '1';
		printmsg(ECU_DBG_CLIENT,data);
		res = send_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
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
	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);

	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
			printmsg(ECU_DBG_CLIENT,"ECUClient_thread_entry Start-------------------------");
	
			ClientThistime = acquire_time();
			//1点或者2点需要清标志位
			if((2 == get_hour())||(1 == get_hour()))
			{
				preprocess();
				resend_record();
				delete_file_resendflag0();		//清空数据resend标志全部为0的目录
			}
			get_time(broadcast_time, broadcast_hour_minute);
			print2msg(ECU_DBG_CLIENT,"time",broadcast_time);
			
			while(search_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				if(1 == flag)		// 还存在需要上传的数据
						data[78] = '1';
				printmsg(ECU_DBG_CLIENT,data);
				res = send_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
				memset(time,0,15);
			}

			delete_file_resendflag0();		//清空数据resend标志全部为0的目录
		
			printmsg(ECU_DBG_CLIENT,"ECUClient_thread_entry End-------------------------");
			
		}
		
		
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
	}
}
