#include "ECUJsonServer.h"
#include <lwip/sockets.h> 
#include "variation.h"
#include <stdio.h>

#define JSON_SERVERPORT 	4570
#define JSON_BACKLOG 		1

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

//�ظ�101����
void Json_Response_101(int connectSocket)
{
	int i = 0;
	char *sendbuff = NULL;
	int length = 0;
	sendbuff = malloc(JSON_RECORD_HEAD + JSON_RECORD_PER_INFO * MAXINVERTERCOUNT);
	memset(sendbuff,'\0',JSON_RECORD_HEAD + JSON_RECORD_PER_INFO * MAXINVERTERCOUNT);
	sprintf(&sendbuff[length],"{\"cmd\":101,\"code\":1,\"ver\":1,\"data\":{\"ecu_id\":\"");
	length = strlen(sendbuff);
	//ECU ID
	sprintf(&sendbuff[length],"%12s",ecu.ECUID12);	
	length = strlen(sendbuff);
	//���������
	sprintf(&sendbuff[length],"\",\"sys_oe\":null");
	length = strlen(sendbuff);
	//���������
	sprintf(&sendbuff[length],",\"sys_ie\":%.2f",ecu.life_energy);
	length = strlen(sendbuff);
	//��ǰ�������豸̨��
	sprintf(&sendbuff[length],",\"dev_no\":%d",ecu.count);
	length = strlen(sendbuff);
	//ʱ���
	if(memcmp(ecu.curTime,"00000000000000",14))
	{
		sprintf(&sendbuff[length],",\"time\":%12s",ecu.curTime);
		length = strlen(sendbuff);
	}
	
	sprintf(&sendbuff[length],",\"opts\":[");
	length = strlen(sendbuff);
	if(memcmp(ecu.curTime,"00000000000000",14))
	{
		for(i = 0;i<ecu.validNum;i++)
		{
			if(inverterInfo[i].status.comm_failed3_status == 1)
			{
				//�豸ID
				sprintf(&sendbuff[length],"{\"dev_id\":\"%12s\"",inverterInfo[i].uid);
				length = strlen(sendbuff);
				//�豸����
				sprintf(&sendbuff[length],",\"type\":1");
				length = strlen(sendbuff);
				//�������
				sprintf(&sendbuff[length],",\"op\":null");
				length = strlen(sendbuff);
				//PV1���빦��
				sprintf(&sendbuff[length],",\"ip1\":%.1f",inverterInfo[i].AveragePower1);
				length = strlen(sendbuff);
				//PV2���빦��
				sprintf(&sendbuff[length],",\"ip2\":%.1f",inverterInfo[i].AveragePower2);
				length = strlen(sendbuff);
				//�������
				sprintf(&sendbuff[length],",\"oe\":null");
				length = strlen(sendbuff);
				//PV1�������
				sprintf(&sendbuff[length],",\"ie1\":%.6f",((float)inverterInfo[i].EnergyPV1)/3600000);
				length = strlen(sendbuff);
				//PV2�������
				sprintf(&sendbuff[length],",\"ie2\":%.6f",((float)inverterInfo[i].EnergyPV2)/3600000);
				length = strlen(sendbuff);
				//�����ѹ
				sprintf(&sendbuff[length],",\"ov\":%.1f",((float)inverterInfo[i].PV_Output)/10);
				length = strlen(sendbuff);
				//PV1�����ѹ
				sprintf(&sendbuff[length],",\"iv1\":%.1f",((float)inverterInfo[i].PV1)/10);
				length = strlen(sendbuff);
				//PV2�����ѹ
				sprintf(&sendbuff[length],",\"iv2\":%.1f",((float)inverterInfo[i].PV2)/10);
				length = strlen(sendbuff);
				//�������
				sprintf(&sendbuff[length],",\"oc\":null");
				length = strlen(sendbuff);
				//PV1�������
				sprintf(&sendbuff[length],",\"ic1\":%.1f",((float)inverterInfo[i].PI)/10);
				length = strlen(sendbuff);
				if(i != ecu.validNum-1)
				{
					//PV2�������
					sprintf(&sendbuff[length],",\"ic2\":null},");
					length = strlen(sendbuff);
				}else
				{
					//PV2�������
					sprintf(&sendbuff[length],",\"ic2\":null}");
					length = strlen(sendbuff);
				}
				
			}
		
		}
	}
	
	sprintf(&sendbuff[length],"]}}");
	length = strlen(sendbuff);

	send(connectSocket,sendbuff,strlen(sendbuff),0);
	free(sendbuff);
	sendbuff = NULL;
}
//�ظ�ʧ������
void Json_Response_failed(int connectSocket)
{
	char *sendbuff = NULL;	
	sendbuff = malloc(200);
	memset(sendbuff,'\0',200);

	memcpy(sendbuff,"{\"code\":0}",10);
	send(connectSocket,sendbuff,10,0);
	lwip_close(connectSocket);
	free(sendbuff);
	sendbuff = NULL;
}


//����ո�ͻ��з�
int clearSpaceBr(char *recvbuff,int length)
{
	int i = 0,j = 0;
	for(i=0;i<length;i++) 
	{
		if((recvbuff[i] != 0x0d)&&(recvbuff[i] != 0x0a)&&(recvbuff[i] != 0x20)&&(recvbuff[i] != 0x09)) 
		{
			recvbuff[j++]=recvbuff[i];
		}			
	}
	recvbuff[j]='\0'; 
	
	return j;
	
}

//������յ�������,���ظ�����
int handle_Recv_Json(int connectSocket,char *recvbuff,int length)
{
	int len = 0;
	
	len = clearSpaceBr(recvbuff,length);
	printf("%d:recv:%s\n",len,recvbuff);
	if(!memcmp(recvbuff,"{\"cmd\":101}",11))
	{
		Json_Response_101(connectSocket);
		return 0;
	}else
	{
		Json_Response_failed(connectSocket);
		return -1;
	}
}

void ECUJsonServer_thread_entry(void* parameter)
{
	int sock, connected;
	rt_uint32_t sin_size=0;
	struct sockaddr_in server_addr, client_addr;
	fd_set rd;
	int recvbytes = 0,ret =0;
	struct timeval timeout;
	char *recvbuff = NULL;	
	int res = 0;
	recvbuff = malloc(200);
	memset(recvbuff,'\0',200);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		/* ����ʧ�ܵĴ����� */
		rt_kprintf("Socket error\n");
		return;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(JSON_SERVERPORT); /* �������˿ں� */
	server_addr.sin_addr.s_addr = INADDR_ANY;
	rt_memset(&(server_addr.sin_zero), 8, sizeof(server_addr.sin_zero));
	
	if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr))	== -1)
	{
		return;	
	}
	/* ��SOCKET�Ͻ��м��� */
	if (listen(sock, JSON_BACKLOG) == -1)
		{
		rt_kprintf("Listen error\n");
		/* release recv buffer */
		return;
	}
		
	rt_kprintf("\nTCPServer Waiting for client on port %d...\n",JSON_SERVERPORT);
	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);
		/* ����һ���ͻ�������socket������,�����������������ʽ�� */
		connected = accept(sock, (struct sockaddr *) &client_addr, &sin_size);
		/* ���ص������ӳɹ���socket */
		/* ���շ��ص�client_addrָ��Ŀͻ��˵�ַ��Ϣ */
		rt_kprintf("I got a connection from (%s , %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		/* �ͻ������Ӵ��� */
		while(1)
		{
			FD_ZERO(&rd);
			FD_SET(connected, &rd);
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;

			res = select(connected+1, &rd, NULL, NULL, &timeout);
			if(res <= 0){
				lwip_close(connected);
				break;
			}
			else{
				memset(recvbuff, '\0', 200);
				recvbytes = recv(connected, recvbuff, 200, 0);
				if(0 == recvbytes) 
					break;
				ret = handle_Recv_Json(connected,recvbuff,recvbytes);
				if(ret == -1){
					break;
				}
			}
		}
	}
	
}
