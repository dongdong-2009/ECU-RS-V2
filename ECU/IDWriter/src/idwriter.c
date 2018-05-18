/*****************************************************************************/
/* File      : idwrite.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwip/sockets.h> 
#include <lwip/netdb.h> 
#include <stdio.h>
#include "debug.h"
#include "idwrite.h"
#include "rtc.h"
#include "inverter_id.h"
#include "threadlist.h"
#include "serverfile.h"
#include <dfs_posix.h> 
#include "channel.h"
#include "led.h"
#include "version.h"
#include "debug.h"
#include "usart5.h"
#include "event.h"
#include "inverter_id.h"
#include "remoteUpdate.h"
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

#define SERVERPORT 4540
#define BACKLOG 2

typedef enum 
{
	UPDATE_VER = 1,
	UPDATE_ID = 2
} eLocalUpdateType;


/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

extern rt_mutex_t record_data_lock;
extern ecu_info ecu;
extern unsigned char LED_IDWrite_Status;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

int create_socket_idwrite(void)
{
	int sockfd;
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		printmsg(ECU_DBG_OTHER,"socket error");
		return -1;
	}
	printmsg(ECU_DBG_OTHER,"Create socket successfully!");
	return sockfd;
}

int bind_socket(int sockfd)
{
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	memset(&(server_sockaddr.sin_zero),0x00,8);

	if(-1==bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))){
		printmsg(ECU_DBG_OTHER,"bind error");
		return -1;
	}
	printmsg(ECU_DBG_OTHER,"Bind socket successfully!");
	return 0;
}

int listen_socket(int sockfd)
{
	if(-1==listen(sockfd,BACKLOG)){
		printmsg(ECU_DBG_OTHER,"listen error");
		return -1;
	}
	printmsg(ECU_DBG_OTHER,"Listen socket successfully!");
	return 0;
}

int accept_socket(int sockfd)
{
	int sin_size;
	int clientfd;
	struct sockaddr_in client_sockaddr;

	sin_size=sizeof(struct sockaddr_in);
	if(-1==(clientfd=accept(sockfd,(struct sockaddr *)&client_sockaddr,(socklen_t *)&sin_size))){
		printmsg(ECU_DBG_OTHER,"accept error");
		return -1;
	}

	return clientfd;
}

int recv_cmd(int fd_sock, char *readbuff)
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, readbytes = 0, res;
	char temp[16];

	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			//printmsg("Receive command timeout");
			return -1;
		}
		else{
			memset(readbuff, '\0', sizeof(readbuff));
			memset(temp, '\0', sizeof(temp));
			recvbytes = recv(fd_sock, readbuff, 200, 0);
			if(0 == recvbytes)
				return -1;
		
			readbytes += recvbytes;
			return readbytes;

		}
	}
}


//��������FTP��������
void ResolveLocarInfo(char *str,eLocalUpdateType* type,char *ip,int *port,char *user,char *password)
{
	int i = 0,j=0,start = 0,end= 0;
	char Type[4] = {'\0'};
	char PortStr[6] = {'\0'};
	for(i = 0;i<=strlen(str);++i)
	{
		if((str[i] == ' ') || (str[i] == '\0') || (str[i] == '\n'))
		{
			end = i;
			if(j == 1)
			{
				memcpy(Type,&str[start],end-start);
			}else if(j == 2)
			{
				memcpy(ip,&str[start],end-start);
			}else if(j == 3)
			{
				memcpy(PortStr,&str[start],end-start);
			}else if(j == 4)
			{
				memcpy(user,&str[start],end-start);
			}else if(j == 5)
			{
				memcpy(password,&str[start],end-start);
			}
			start = i+1;
			j++;
		}
	}

	if(!memcmp(Type,"VER",3))
	{
		*type = UPDATE_VER;
	}else{
		*type = UPDATE_ID;
	}
	
	*port =atoi(PortStr);
}


int settime(char *datetime)
{
	return set_time(datetime);
}

int insertinverter(char *buff)
{

	FILE *fp;
	char id[16];
	int i;

	clear_id();
	for(i=0; i<(strlen(buff)+1)/13; i++)
	{

		strncpy(id, &buff[i*13], 12);
		id[12] = '\0';
		addInverter(id);
	}

	fp = fopen("/config/autoflag.con", "w");
	fputs("0", fp);
	fclose(fp);
	fp = fopen("/config/limiteid.con", "w");
	fputs("1", fp);
	fclose(fp);
	restartThread(TYPE_DATACOLLECT);

	return i;
}

int getrecord(char *record)
{
	char time[15];
	int flag;
	return search_readflag(record,time, &flag,'1');
}

int getevent(char *eve)
{
	FILE *fp;
	char data[200];
	char splitdata[3][32];
	int num = 0;
	
	memset(eve,'\0',sizeof(eve));
	fp = fopen("/home/record/event", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,3*32);
			splitString(data,splitdata);
			strcat(eve, splitdata[0]);	//12
			strcat(eve, splitdata[1]);	//14
			strcat(eve, splitdata[2]);	//24
			num++;
		}
		fclose(fp);
	}

	return num;
}

int clearrecord()
{
	unlink("/config/AUTOFLAG.CON");
	unlink("/config/LIMITEID.CON");
	unlink("/home/data/COLLECT.CON");

	clear_id();

	echo("/home/data/ltpower","0.000000");	

	rm_dir("/home/record/data");
	rm_dir("/home/record/tridata");
	rm_dir("/home/record/POWER");
	rm_dir("/home/record/ENERGY");
	rm_dir("/home/record/CTLDATA");
	rm_dir("/home/record/ALMDATA");
	rm_dir("/home/record/RSDINFO");

	rm_dir("/tmp");
	restartThread(TYPE_DATACOLLECT);
	return 0;
}

void idwrite_thread_entry(void* parameter)
{
	char recvbuff[200] = {'\0'};
	int sockfd,clientfd;
	FILE *fp;
	char mac[32] = {'\0'};
	char version[50] = {'\0'};
	char area[8] = {'\0'};
	char *record;
	int row;
	char sendbuff[3];
	char gettime[14]={'\0'};
	int ret = 0;
	
	rt_thread_delay(START_TIME_IDWRITE * RT_TICK_PER_SECOND);
	sockfd=create_socket_idwrite();
	bind_socket(sockfd);
	listen_socket(sockfd);

	while(1){
		clientfd = accept_socket(sockfd);
		memset(recvbuff, '\0', sizeof(recvbuff));
		
		recv_cmd(clientfd, recvbuff);
		print2msg(ECU_DBG_OTHER,"recvbuff",recvbuff);
		rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
		if(!strncmp(recvbuff, "mkfs", 4))
		{
			dfs_mkfs("elm","flash");
			initPath();
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,"mkfs OK\n",8,0));
		}
		
		//��д�Ͷ�ȡECU��ID
		if(!strncmp(recvbuff, "set_ecu_id", 10)){
			strncpy(ecu.ECUID12, &recvbuff[11], 12);
			print2msg(ECU_DBG_OTHER,"ECU id",ecu.ECUID12);
			printdecmsg(ECU_DBG_OTHER,"length",strlen(ecu.ECUID12));
			ecu.ECUID12[12] = '\0';
			ret = 0;
			if(InitWorkMode() < 0){
				ret = -1;
			}
			if(setECUID(ecu.ECUID12) < 0){
				ret = -2;
			}
			
			fp=fopen("/config/ecuid.con","r");
			fgets(ecu.ECUID12,13,fp);
			fclose(fp);
			restartThread(TYPE_DATACOLLECT);
			if(ret == 0)
			{
				printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,ecu.ECUID12,strlen(ecu.ECUID12),0));
			}else if (ret == -1)
			{
				printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,"error code: -1",14,0));
			}else if (ret == -2)
			{
				printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,"error code: -2",14,0));
			}
		}
		if(!strncmp(recvbuff, "get_ecu_id", 10)){
			memset(ecu.ECUID12,'\0',sizeof(ecu.ECUID12));
			fp=fopen("/config/ecuid.con","r");
			fgets(ecu.ECUID12,13,fp);
			fclose(fp);
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,ecu.ECUID12,strlen(ecu.ECUID12),0));
		}

		//��д�Ͷ�ȡECU���������MAC
		if(!strncmp(recvbuff, "set_eth0_mac", 12)){
			mac[0] = recvbuff[13];
			mac[1] = recvbuff[14];
			mac[2] = ':';
			mac[3] = recvbuff[15];
			mac[4] = recvbuff[16];
			mac[5] = ':';
			mac[6] = recvbuff[17];
			mac[7] = recvbuff[18];
			mac[8] = ':';
			mac[9] = recvbuff[19];
			mac[10] = recvbuff[20];
			mac[11] = ':';
			mac[12] = recvbuff[21];
			mac[13] = recvbuff[22];
			mac[14] = ':';
			mac[15] = recvbuff[23];
			mac[16] = recvbuff[24];
			print2msg(ECU_DBG_OTHER,"ECU eth0 MAC address",mac);
			printdecmsg(ECU_DBG_OTHER,"length",strlen(mac));
			fp=fopen("/config/ecumac.con","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			
			fp=fopen("/config/ecumac.con","r");
			fgets(mac,18,fp);
			fclose(fp);
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_eth0_mac", 12)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/config/ecumac.con","r");
			fgets(mac,18,fp);
			fclose(fp);
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd,mac,strlen(mac),0));
		}

		//����ECU�ı���ʱ��
		if(!strncmp(recvbuff, "set_time", 8)){
			settime(&recvbuff[9]);
			send(clientfd, &recvbuff[9], 14, 0);
			restartThread(TYPE_DATACOLLECT);
		}

		//�����������ID
		if(!strncmp(recvbuff, "set_inverter_id", 15)){
			
			if(1 == saveECUChannel(18))
				send(clientfd, "0x12", 4, 0);
			else
				send(clientfd, "change channel failed", 21, 0);
			
			row = insertinverter(&recvbuff[16]);
			snprintf(sendbuff, sizeof(sendbuff), "%02d", row);
			send(clientfd, sendbuff, 3, 0);
			restartThread(TYPE_DATACOLLECT);
		}

		//��ȡPLC�Ĳ��Խ��
		if(!strncmp(recvbuff, "query_result", 12)){
			record = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
			if(getrecord(record) > 0)
				send(clientfd, record, strlen(record), 0);
			else
				send(clientfd, "Failed", 6, 0);
			free(record);
		}

		//��ղ��Լ�¼
		if(!strncmp(recvbuff, "clear", 5)){

			if(1 == saveECUChannel(0x17))
				send(clientfd, "0x17", 4, 0);
			else
				send(clientfd, "change channel failed", 21, 0);
			clearrecord();
			//system("rm /home/tmpdb");
			//system("rm /home/historical_data.db");
			//system("rm /home/record.db");
			send(clientfd, "clearok", 7, 0);
		}


		//��ȡECU����汾��
		if(!strncmp(recvbuff, "get_version", 11)){
			memset(version, 0, sizeof(version));
			sprintf(version,"%s_%s.%s", ECU_VERSION,MAJORVERSION,MINORVERSION);
			memset(area, 0, sizeof(area));
			fp = fopen("/config/area.con", "r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			strcat(version, area);
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd, version, strlen(version), 0));
		}

		//��ȡ����ʱ��
		if(!strncmp(recvbuff, "get_time", 8)){
			apstime(gettime);
			printdecmsg(ECU_DBG_OTHER,"Send",send(clientfd, gettime, 14, 0));
		}
		//��������
		if(!strncmp(recvbuff, "reboot", 6)){
			send(clientfd, "reboot ok", 9, 0);
			closesocket(clientfd);
			reboot();
		}

		//����������Ҫ�����FTP
		if(!strncmp(recvbuff, "update", 6)){
			eLocalUpdateType type;
			char ip[25] = {'\0'};
			int port = 0;
			char user[20] = {'\0'};
			char password[20] = {'\0'};

			ResolveLocarInfo(recvbuff,&type,ip,&port,user,password);
			
			if(UPDATE_VER == type)
			{
				ret =updateECUByVersion_Local("",ip,port,user,password);

			}else
			{
				ret =updateECUByID_Local("",ip,port,user,password);
			}

			if(ret == 0)
			{
				send(clientfd, "update ok", 9, 0);
				closesocket(clientfd);
				reboot();
			}	
			else if(ret == 550)
				send(clientfd, "update file not exist", 21, 0);
			else
				send(clientfd, "update failed", 13, 0);
			
		}

		//��ȡFlash�ռ�
		if(!strncmp(recvbuff, "get_flash", 9)){
			char flash[10] ={'\0'};
			long long cap;
			struct statfs buffer;
			dfs_statfs("/", &buffer);
			cap = buffer.f_bsize * buffer.f_bfree / 1024;
			sprintf(flash,"%d",(unsigned int)cap);
			send(clientfd, flash, strlen(flash), 0);
		}
		//��ȡ�ڲ��汾��
		if(!strncmp(recvbuff, "Internal_version", 16)){
			char version[10] ={'\0'};
			sprintf(version,"%d",(unsigned int)INTERNAL_TEST_VERSION);
			send(clientfd, version, strlen(version), 0);
		}

		//����LED��
		if(!strncmp(recvbuff, "test_led", 8)){
			rt_hw_led_init();	//LED��ʼ��
			rt_hw_ms_delay(500);
			rt_hw_led_on();		//LED��
			rt_hw_ms_delay(500);
			rt_hw_led_off();		//LEDϨ��
			rt_hw_ms_delay(500);
			rt_hw_led_on();		//LED��
			rt_hw_ms_delay(500);
			rt_hw_led_off();		//LEDϨ��
			rt_hw_ms_delay(500);
			rt_hw_led_on();		//LED��
			rt_hw_ms_delay(500);
			rt_hw_led_off();		//LEDϨ��
			LED_IDWrite_Status = 1;
		}
		rt_mutex_release(record_data_lock);
		closesocket(clientfd);
	}
}
