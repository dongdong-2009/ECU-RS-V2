#include "ECUControl.h"
#include "usart5.h"
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

#include "set_rsd_function_switch.h"
#include "inverter_id.h"
#include "custom_command.h"
#include <dfs_posix.h> 

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150, A151, A152, A153, A154, A155, A156, A157, A158, A159,
	A160, A161, A162, A163, A164, A165, A166, A167, A168, A169,
};
int (*pfun[200])(const char *recvbuffer, char *sendbuffer);

void add_functions()
{
	pfun[A102] = response_inverter_id; 			//�ϱ������ID  										OK
	pfun[A103] = set_inverter_id; 				//���������ID												OK
	pfun[A108] = custom_command;				//��ECU�����Զ�������
	pfun[A160] = set_rsd_function_switch; 			//RSD���ܿ���									OK

}

/* ��EMA����ͨѶ */
int communication_with_EMA(int next_cmd_id)
{
	int sockfd;
	int cmd_id;
	char timestamp[15] = "00000000000000";	//ʱ���
	char *recv_buffer = NULL;
	char *send_buffer = NULL;

	recv_buffer = rt_malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	send_buffer = rt_malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	
	while(1)
	{
		printmsg(ECU_DBG_CONTROL_CLIENT,"Start Communication with EMA");
		sockfd = Control_client_socket_init();
		if(sockfd < 0) 
		{
		
#ifdef WIFI_USE	

			//��������ʧ�ܣ�ʹ��wifi���� 
			{
				int j =0,flag_failed = 0,ret = 0;
				if(next_cmd_id <= 0)
				{
					//ECU��EMA������������ָ��
					msg_REQ(send_buffer);
					ret = SendToSocketC(send_buffer, strlen(send_buffer));
					if(ret == -1)
					{
						rt_free(recv_buffer);
						rt_free(send_buffer);
						return -1;
					}
					memset(send_buffer, '\0', sizeof(send_buffer));
					for(j = 0;j<800;j++)
					{
						if(WIFI_Recv_SocketC_Event == 1)
						{
							flag_failed = 1;
							WIFI_Recv_SocketC_Event = 0;
							break;
						}
						rt_hw_ms_delay(10);
					}
					if(flag_failed == 0)
					{
						rt_free(recv_buffer);
						rt_free(send_buffer);
						return -1;
					}

					//ȥ��usr WIFI���ĵ�ͷ��
					memcpy(recv_buffer,WIFI_RecvSocketCData,WIFI_Recv_SocketC_LEN);
					recv_buffer[WIFI_Recv_SocketC_LEN] = '\0';
					print2msg(ECU_DBG_CONTROL_CLIENT,"communication_with_EMA recv",recv_buffer);
					//У������
					if(msg_format_check(recv_buffer) < 0){
						continue;
					}
					//���������
					cmd_id = msg_cmd_id(recv_buffer);
				}
				else{
					//������һ������(�����������������,�ϱ����ú��ECU״̬)
					cmd_id = next_cmd_id;
					next_cmd_id = 0;
					memset(recv_buffer, 0, sizeof(recv_buffer));
					snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
							ecu.ECUID12, cmd_id, timestamp);
				}

				//��������ŵ��ú���
				if(pfun[cmd_id%100]){
					//�����ú���������Ϻ���Ҫִ���ϱ�,��᷵���ϱ������������,���򷵻�0
					next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
				}
				//EMA��������
				else if(cmd_id == 100){
					break;
				}
				else{
					//������Ų�����,��������ʧ��Ӧ��(ÿ������Э���ʱ���λ�ò�ͳһ,����ʱ����Ǹ�����...)
					memset(send_buffer, 0, sizeof(send_buffer));
					snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END\n",
							ecu.ECUID12, cmd_id);
				}
				//����Ϣ���͸�EMA(�Զ����㳤��,���ϻس�)
				SendToSocketC(send_buffer, strlen(send_buffer));
				printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
				//������ܺ�������ֵС��0,�򷵻�-1,������Զ��˳�
				if(next_cmd_id < 0){
					rt_free(recv_buffer);
					rt_free(send_buffer);
					return -1;
				}				
			}
#endif

#ifndef WIFI_USE
		break;
#endif
	
		}
		else
		{
			if(next_cmd_id <= 0)
			{
				//ECU��EMA������������ָ��
				msg_REQ(send_buffer);
				send_socket(sockfd, send_buffer, strlen(send_buffer));
				memset(send_buffer, '\0', sizeof(send_buffer));

				//����EMA����������
				if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), 10) < 0){
					close_socket(sockfd);
					rt_free(recv_buffer);
					rt_free(send_buffer);
					return -1;
				}
				//У������
				if(msg_format_check(recv_buffer) < 0){
					close_socket(sockfd);
					continue;
				}
				//���������
				cmd_id = msg_cmd_id(recv_buffer);
			}
			else{
				//������һ������(�����������������,�ϱ����ú��ECU״̬)
				cmd_id = next_cmd_id;
				next_cmd_id = 0;
				memset(recv_buffer, 0, sizeof(recv_buffer));
				snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
						ecu.ECUID12, cmd_id, timestamp);
			}

			if(pfun[cmd_id%100]){
				//�����ú���������Ϻ���Ҫִ���ϱ�,��᷵���ϱ������������,���򷵻�0
				next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
			}
			//EMA��������
			else if(cmd_id == 100){
				close_socket(sockfd);
				break;
			}
			else{
				//������Ų�����,��������ʧ��Ӧ��(ÿ������Э���ʱ���λ�ò�ͳһ,����ʱ����Ǹ�����...)
				memset(send_buffer, 0, sizeof(send_buffer));
				snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END",
						ecu.ECUID12, cmd_id);
			}
			//����Ϣ���͸�EMA(�Զ����㳤��,���ϻس�)
			send_socket(sockfd, send_buffer, strlen(send_buffer));
			printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
			close_socket(sockfd);

			//������ܺ�������ֵС��0,�򷵻�-1,������Զ��˳�
			if(next_cmd_id < 0){
				rt_free(recv_buffer);
				rt_free(send_buffer);
				return -1;
			}
		}
	}
	
	printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
	rt_free(recv_buffer);
	rt_free(send_buffer);
	return 0;
}

//����A157���� 
int prealarmprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_alarm_resendflag2())		//	����Ƿ���resendflag='2'�ļ�¼
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS1300047A157AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//���͵�������
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes > 54)
	{
		clear_alarm_send_flag(&readbuff[49]);
		
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

int send_alarm_record(char *sendbuff, char *send_date_time)			//�������ݵ�EMA  ע���ڴ洢��ʱ���βδ���'\n'  �ڷ���ʱ��ʱ��ǵ����
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
		if('1' == readbuff[49])
			update_alarm_send_flag(send_date_time);
		clear_alarm_send_flag(&readbuff[49]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_alarm_record()
{
	char *data = NULL;//��ѯ��������
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//��/home/record/data/Ŀ¼�²�ѯresendflagΪ2�ļ�¼
	while(search_alarm_readflag(data,time,&flag,'2'))		//	��ȡһ��resendflagΪ1������
	{
		//printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_alarm_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}

//����A159���� 
int precontrolprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_control_resendflag2())		//	����Ƿ���resendflag='2'�ļ�¼
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS1300047A159AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//���͵�������
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes > 54)
	{
		clear_control_send_flag(&readbuff[49]);
		
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

int send_control_record(char *sendbuff, char *send_date_time)			//�������ݵ�EMA  ע���ڴ洢��ʱ���βδ���'\n'  �ڷ���ʱ��ʱ��ǵ����
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
		if('1' == readbuff[49])
			update_control_send_flag(send_date_time);
		clear_control_send_flag(&readbuff[49]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_control_record()
{
	char *data = NULL;//��ѯ��������
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//��/home/record/data/Ŀ¼�²�ѯresendflagΪ2�ļ�¼
	while(search_control_readflag(data,time,&flag,'2'))		//	��ȡһ��resendflagΪ1������
	{
		//printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_control_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}


//���߳���Ҫ���������ϴ��Լ�Զ�̿���
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	int AlarmThistime=0, AlarmDurabletime=65535, AlarmReportinterval=180;
	char *data = NULL;
	int res,flag,result;
	FILE *fp;
	char time[15] = {'\0'};
	//��ӹ��ܺ���
  	add_functions();
	printmsg(ECU_DBG_CONTROL_CLIENT,"add functions Over");
	rt_thread_delay(RT_TICK_PER_SECOND*START_TIME_CONTROL_CLIENT);
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);

	while(1)
	{
		//Զ�̿����ϱ�
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//Զ�̿��� 15�����ϱ�	
			printmsg(ECU_DBG_CONTROL_CLIENT,"Control DATA Start");
			ControlDurabletime = acquire_time();
			ControlThistime = acquire_time();
			
			if(22 == get_hour())
			{
				precontrolprocess();
				resend_control_record();
				delete_control_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
			}

			while(search_control_readflag(data,time,&flag,'1'))		//	��ȡһ��resendflagΪ1������
			{
				if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
				{
						break;
				}
				//printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_control_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			delete_control_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
			fp=fopen("/TMP/ECUUPVER.CON","r");
			if(fp!=NULL)
			{
				char c='0';
				c=fgetc(fp);
				fclose(fp);
				if(c=='1')
				{
					printf("111111111111\n");
					result = communication_with_EMA(102);
					if(result != -1)
					{
						unlink("/TMP/ECUUPVER.CON");
					}
					
				}
			}else
			{
				communication_with_EMA(0);
			}
			

			printmsg(ECU_DBG_CONTROL_CLIENT,"Control DATA End");

		}
		
		
		//�ϱ��澯��־
		if(compareTime(AlarmDurabletime ,AlarmThistime,AlarmReportinterval))
		{			
			printmsg(ECU_DBG_CONTROL_CLIENT,"Alarm DATA Start");
			AlarmDurabletime = acquire_time();
			AlarmThistime = acquire_time();

			if(23 == get_hour())
			{
				//����A157 ����쳣��־����
				prealarmprocess();
				//�ط���־λΪ2�ı�־
				resend_alarm_record();
				delete_alarm_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
			}

			while(search_alarm_readflag(data,time,&flag,'1'))		//	��ȡһ��resendflagΪ1������
			{
				if(compareTime(AlarmDurabletime ,AlarmThistime,AlarmReportinterval))
				{
						break;
				}
				//printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_alarm_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			delete_alarm_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
			printmsg(ECU_DBG_CONTROL_CLIENT,"Alarm DATA End");
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();		
		AlarmDurabletime = acquire_time();

	}
	
}


#ifdef RT_USING_FINSH
#include <finsh.h>
void commEMA(void)
{
	communication_with_EMA(0);
}
FINSH_FUNCTION_EXPORT(commEMA, eg:commEMA());
#endif

