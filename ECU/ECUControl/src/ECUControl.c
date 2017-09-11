#include "ECUControl.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"
#include "debug.h"
#include "threadlist.h"
#include "serverfile.h"
#include "remote_control_protocol.h"
#include "string.h"
#include "stdlib.h"
#include "rtc.h"
#include "socket.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150, A151, A152, A153, A154, A155, A156, A157, A158, A159,
};
int (*pfun[100])(const char *recvbuffer, char *sendbuffer);

void add_functions()
{
  //pfun[A102] = response_inverter_id; 			//上报逆变器ID  										OK

}

//发送A157命令 
int prealarmprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_alarm_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS16AAAAAA157AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		return -1;
	}
	if(readbytes > 54)
	{
		clear_alarm_send_flag(&readbuff[48]);
		
	}else
	{
		return -1;
	}
	return 0;
}

int send_alarm_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	char *readbuff = NULL;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,'\0',(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes < 54)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		if('3' == readbuff[48])
			update_alarm_send_flag(send_date_time);
		clear_alarm_send_flag(&readbuff[48]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_alarm_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_alarm_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_alarm_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}

//发送A159命令 
int precontrolprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_control_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS16AAAAAA159AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		return -1;
	}
	if(readbytes > 54)
	{
		clear_control_send_flag(&readbuff[48]);
		
	}else
	{
		return -1;
	}
	return 0;
}

int send_control_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	char *readbuff = NULL;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,'\0',(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes < 54)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		if('3' == readbuff[48])
			update_control_send_flag(send_date_time);
		clear_control_send_flag(&readbuff[48]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_control_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_control_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_control_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}


//该线程主要用于数据上传以及远程控制
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	int AlarmThistime=0, AlarmDurabletime=65535, AlarmReportinterval=120;
	char *data = NULL;
	int res,flag;
	char time[15] = {'\0'};
	
	rt_thread_delay(RT_TICK_PER_SECOND*START_TIME_CONTROL_CLIENT);
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	//添加功能函数
  	add_functions();
	
	while(1)
	{
		//远程控制上报
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//远程控制 15分钟上报	
			printmsg(ECU_DBG_CONTROL_CLIENT,"Control DATA Start");

			ControlThistime = acquire_time();

			if((2 == get_hour())||(1 == get_hour()))
			{
				precontrolprocess();
				resend_control_record();
				delete_control_file_resendflag0();		//清空数据resend标志全部为0的目录
			}

			while(search_control_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_control_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			
			printmsg(ECU_DBG_COLLECT,"Control DATA End");

		}
		
		//上报告警标志
		if(compareTime(AlarmDurabletime ,AlarmThistime,AlarmReportinterval))
		{			
			printmsg(ECU_DBG_CONTROL_CLIENT,"Alarm DATA Start");
			AlarmThistime = acquire_time();

			if((2 == get_hour())||(1 == get_hour()))
			{
				//发送A157 情况异常标志命令
				prealarmprocess();
				//重发标志位为2的标志
				resend_alarm_record();
				delete_alarm_file_resendflag0();		//清空数据resend标志全部为0的目录
			}

			while(search_alarm_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_alarm_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			
			printmsg(ECU_DBG_COLLECT,"Alarm DATA End");
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();		
		AlarmDurabletime = acquire_time();

	}
}
