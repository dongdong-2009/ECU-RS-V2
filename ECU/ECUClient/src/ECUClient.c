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
#include "usart5.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern rt_mutex_t usr_wifi_lock;

//����ͷ��Ϣ��EMA,��ȡ�Ѿ�����EMA�ļ�¼ʱ��
int preprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char sendbuff[50] = {'\0'};

	if(0 == detection_resendflag2())		//	����Ƿ���resendflag='2'�ļ�¼
		return 0;
	readbuff = malloc((4+99*14));
	memset(readbuff,0x00,(4+99*14));
	readbytes = 4+99*14;
	strcpy(sendbuff, "APS13AAA22");
	memcpy(&sendbuff[10],ecu.ECUID12,12);
	strcat(sendbuff, "\n");
	print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

	//���͵�������
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

int send_record(int fd_sock, char *sendbuff, char *send_date_time)			//�������ݵ�EMA  ע���ڴ洢��ʱ���βδ���'\n'  �ڷ���ʱ��ʱ��ǵ����
{
	int sendbytes=0;
	char *readbuff = NULL;
	int send_length = 0,length;
	readbuff = malloc((4+99*14));
	memset(readbuff,'\0',(4+99*14));
	length = strlen(sendbuff);
	while(length > 0)
	{
		if(length > SIZE_PER_SEND)
		{
			printf("length :%d\n",length);
			sendbytes = send(fd_sock, &sendbuff[send_length], SIZE_PER_SEND, 0);
			send_length += SIZE_PER_SEND;
			length -= SIZE_PER_SEND;
		}else
		{	
			printf("length :%d\n",length);
			sendbytes = send(fd_sock, &sendbuff[send_length], length, 0);

			length -= length;
		}

		if(-1 == sendbytes)
		{
			printf("11111111:%d\n",errno);
			free(readbuff);
			readbuff = NULL;
			return -1;
		}
		
		rt_hw_ms_delay(500);
	}

	
	

	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(-1 == recv_response(fd_sock, readbuff))
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		print2msg(ECU_DBG_CLIENT,"readbuff",readbuff);
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
	
}

#ifdef WIFI_USE
int wifi_send_record(char *sendbuff, char *send_date_time)		//ͨ��WIFI�������ݵ�EMA  ע���ڴ洢��ʱ���βδ���'\n'  �ڷ���ʱ��ʱ��ǵ����
{
	int j = 0,ret = 0;
	rt_mutex_take(usr_wifi_lock, RT_WAITING_FOREVER);
	ret = SendToSocketB(sendbuff, strlen(sendbuff));
	if(ret == -1)
	{
		rt_mutex_release(usr_wifi_lock);
		return -1;
	}
	
	for(j = 0;j<800;j++)
	{
		if(WIFI_Recv_SocketB_Event == 1)
		{
			//����ʽ������Ƕ������һ���ˣ����кϲ�
			wifi_socketb_format((char *)WIFI_RecvSocketBData ,WIFI_Recv_SocketB_LEN);
			print2msg(ECU_DBG_CLIENT,"readbuff",(char *)WIFI_RecvSocketBData);
			if('1' == WIFI_RecvSocketBData[0])
				update_send_flag(send_date_time);
			clear_send_flag((char *)WIFI_RecvSocketBData);
			WIFI_Recv_SocketB_Event = 0;
			rt_mutex_release(usr_wifi_lock);
			return 0;
		}
		rt_hw_ms_delay(10);
	}
	rt_mutex_release(usr_wifi_lock);
	return -1;

}
#endif

int resend_record()
{
	int fd_sock;
	char *data = NULL;//��ѯ��������
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);

	//��/home/record/data/Ŀ¼�²�ѯresendflagΪ2�ļ�¼
	fd_sock = createsocket();
	printdecmsg(ECU_DBG_CLIENT,"Socket", fd_sock);
	if(1 == connect_client_socket(fd_sock))
	{
		while(search_readflag(data,time,&flag,'2'))		//	��ȡһ��resendflagΪ1������
		{
			if(1 == flag)		// ��������Ҫ�ϴ�������
					data[88] = '1';
			printmsg(ECU_DBG_CLIENT,data);
			res = send_record(fd_sock,data, time);
			if(-1 == res)
				break;
		}
		close_socket(fd_sock);
#ifdef WIFI_USE 
	}else
	{
		while(search_readflag(data,time,&flag,'2'))
		{
			if(1 == flag)		// ��������Ҫ�ϴ�������
				data[88] = '1';
			printmsg(ECU_DBG_CLIENT,data);
			res = wifi_send_record(data, time);
			if(-1 == res)
				break;
		}
#endif
	}
	free(data);
	data = NULL;
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
	int fd_sock;
	char time[15] = {'\0'};
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_CLIENT);
	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);

	while(1)
	{
		if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval)){
			printmsg(ECU_DBG_CLIENT,"Client Start");
			ClientDurabletime = acquire_time();
			ClientThistime = acquire_time();
			//1�����2����Ҫ���־λ
			if((2 == get_hour())||(1 == get_hour()))
			{
				preprocess();
				resend_record();
				delete_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
			}
			get_time(broadcast_time, broadcast_hour_minute);
			print2msg(ECU_DBG_CLIENT,"time",broadcast_time);

			fd_sock = createsocket();
			printdecmsg(ECU_DBG_CLIENT,"Socket", fd_sock);
			
			if(1 == connect_client_socket(fd_sock))
			{
				while(search_readflag(data,time,&flag,'1'))		//	��ȡһ��resendflagΪ1������
				{
					if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval))
					{
						break;
					}
					if(1 == flag)		// ��������Ҫ�ϴ�������
							data[88] = '1';
					printmsg(ECU_DBG_CLIENT,data);
					res = send_record(fd_sock,data, time);
					if(-1 == res)
						break;
					ClientThistime = acquire_time();
					memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
					memset(time,0,15);
				}
				close_socket(fd_sock);
#ifdef WIFI_USE
			}else
			{
				//ͨ�����ߴ�������
				{
					while(search_readflag(data,time,&flag,'1')) 	//	��ȡһ��resendflagΪ1������
					{
						if(compareTime(ClientDurabletime ,ClientThistime,ClientReportinterval))
						{	
							break;
						}
						if(1 == flag)		// ��������Ҫ�ϴ�������
							data[88] = '1';
						printmsg(ECU_DBG_CLIENT,data);
						res = wifi_send_record(data, time);
						if(-1 == res)
						{
							break;
						}
									
						ClientThistime = acquire_time();
						memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
						memset(time,0,15);
					}
				}
				WIFI_Close(SOCKET_B);
#endif
			}

			delete_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
		
			printmsg(ECU_DBG_CLIENT,"Client End");
			
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND);
		ClientDurabletime = acquire_time();		
	}
}
