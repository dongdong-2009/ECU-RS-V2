#include "socket.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "lan8720rst.h"
#include <stdio.h>
#include "rtc.h"
#include "threadlist.h"
#include "usart5.h"

#define CLIENT_SERVER_DOMAIN	""
#define CLIENT_SERVER_IP			"192.168.1.103"
#define CLIENT_SERVER_PORT1	8093
#define CLIENT_SERVER_PORT2	8093

#define CONTROL_SERVER_DOMAIN	""
#define CONTROL_SERVER_IP			"192.168.1.103"
#define CONTROL_SERVER_PORT1	8997
#define CONTROL_SERVER_PORT2	8997

int createsocket(void)					//创建SOCKET连接
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printf("Failed to create socket\n");
	else
		printf("Create socket successfully\n");

	return fd_sock;
}

int connect_client_socket(int fd_sock)				//通过有线的方式连接服务器
{
	char domain[100]={'\0'};		//服务器域名
	char ip[20] = {'\0'};	//服务器IP地址
	int port[2]={CLIENT_SERVER_PORT1, CLIENT_SERVER_PORT2};	//服务器端口号
	struct sockaddr_in serv_addr;
	struct hostent * host;

	//不存在有线连接，直接关闭socket
	if(rt_hw_GetWiredNetConnect() == 0)
	{
		closesocket(fd_sock);
		return -1;
	}
	strcpy(domain, CLIENT_SERVER_DOMAIN);
	strcpy(ip, CLIENT_SERVER_IP);


	host = gethostbyname(domain);
	if(NULL == host)
	{
		printf("Resolve domain failure\n");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		sprintf(ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
	}

	printf("IP:%s\n", ip);
	printf("Port1:%d\n", port[0]);
	printf("Port2:%d\n", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[0]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		printf("Failed to connect to EMA\n");
		closesocket(fd_sock);
		return -1;
	}
	else{
		printf("Connect to EMA Client successfully\n");
		return 1;
	}
}

int connect_control_socket(int fd_sock)				//通过有线的方式连接服务器
{
	char domain[100]={'\0'};		//服务器域名
	char ip[20] = {'\0'};	//服务器IP地址
	int port[2]={CONTROL_SERVER_PORT1, CONTROL_SERVER_PORT2};	//服务器端口号
	struct sockaddr_in serv_addr;
	struct hostent * host;

	//不存在有线连接，直接关闭socket
	if(rt_hw_GetWiredNetConnect() == 0)
	{
		closesocket(fd_sock);
		return -1;
	}
	strcpy(domain, CONTROL_SERVER_DOMAIN);
	strcpy(ip, CONTROL_SERVER_IP);


	host = gethostbyname(domain);
	if(NULL == host)
	{
		printf("Resolve domain failure\n");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		sprintf(ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
	}

	printf("IP:%s\n", ip);
	printf("Port1:%d\n", port[0]);
	printf("Port2:%d\n", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[0]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		printf("Failed to connect to EMA\n");
		closesocket(fd_sock);
		return -1;
	}
	else{
		printf("Connect to EMA Client successfully\n");
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭socket连接
{
	closesocket(fd_sock);
	printf("Close socket\n");
}


int wifi_socketb_format(char *data ,int length)
{
	char head[9] = {'\0'};
	char *p = NULL;
	int i = 0,retlength = 0;
	/*
	head[0] = 0x62;
	head[1] = 0x00;
	head[2] = 0x00;
	head[3] = 0x00;
	head[4] = 0x00;
	head[5] = 0x00;
	head[6] = 0x00;
	head[7] = 0x00;
	head[8] = 0x00;
	*/
	head[0] = 'b';
	head[1] = '0';
	head[2] = '0';
	head[3] = '0';
	head[4] = '0';
	head[5] = '0';
	head[6] = '0';
	head[7] = '0';
	head[8] = '0';
	for(p = data,i = 0;p <= (data+length-9);p++,i++)
	{
		if(!memcmp(p,head,9))
		{
			memcpy(p,p+9,(length-9-i));
			length -= 9;
			data[length] = '\0';
			retlength = length;
		}
	}

	return retlength;
}
//与Client服务器通讯 
//sendbuff[in]:发送数据的buff
//sendLength[in]:发送字节长度
//recvbuff[out]:接收数据buff
//recvLength[in/out]:in接收数据最大长度  out接收数据长度
//Timeout[in]	超时时间 ms
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout)
{
	int socketfd = 0;
	
	socketfd = createsocket();
	if(socketfd == -1) 	//创建socket失败
		return -1;
	//创建socket成功
	if(1 == connect_client_socket(socketfd))
	{	//连接服务器成功
		int sendbytes = 0;
		int res = 0;
		fd_set rd;
		struct timeval timeout;
		sendbytes = send(socketfd, sendbuff, sendLength, 0);
		if(-1 == sendbytes)
		{
			close_socket(socketfd);
			return -1;
		}
		FD_ZERO(&rd);
		FD_SET(socketfd, &rd);
		timeout.tv_sec = Timeout/1000;
		timeout.tv_usec = Timeout%1000;
		res = select(socketfd+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			printf("Receive data reply from Server timeout\n");
			close_socket(socketfd);
			return -1;
		}else
		{
			*recvLength = recv(socketfd, recvbuff, *recvLength, 0);
			if(0 == *recvLength)
			{
				close_socket(socketfd);
				return -1;
			}else
			{
				printf("serverCommunication_Client:%s\n",recvbuff);
				close_socket(socketfd);
				return 0;
			}
		}
		
	}else
	{
		//连接服务器失败
		//失败情况下通过无线发送
#ifdef WIFI_USE
		int ret = 0,i = 0;
		ret = SendToSocketB(sendbuff, sendLength);
		if(ret == -1)
			return -1;
		for(i = 0;i<(Timeout/10);i++)
		{
			if(WIFI_Recv_SocketB_Event == 1)
			{
				*recvLength = wifi_socketb_format((char *)WIFI_RecvSocketBData ,WIFI_Recv_SocketB_LEN);
				memcpy(recvbuff,WIFI_RecvSocketBData,*recvLength);
				recvbuff[*recvLength] = '\0';
					printf("serverCommunication_Client:%s\n",recvbuff);
				WIFI_Recv_SocketB_Event = 0;
				WIFI_Close(SOCKET_B);
				return 0;
			}
			rt_hw_ms_delay(10);
		}
		WIFI_Close(SOCKET_B);
		return -1;
#endif

#ifndef WIFI_USE
		return -1;
#endif
	}
	
}


//与Control服务器通讯 
//sendbuff[in]:发送数据的buff
//sendLength[in]:发送字节长度
//recvbuff[out]:接收数据buff
//recvLength[in/out]:in接收数据最大长度  out接收数据长度
//Timeout[in]	超时时间 ms
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout)
{
	int socketfd = 0;
	
	socketfd = createsocket();
	if(socketfd == -1) 	//创建socket失败
		return -1;
	//创建socket成功
	if(1 == connect_control_socket(socketfd))
	{	//连接服务器成功
		int sendbytes = 0;
		int res = 0;
		fd_set rd;
		struct timeval timeout;
		sendbytes = send(socketfd, sendbuff, sendLength, 0);
		if(-1 == sendbytes)
		{
			close_socket(socketfd);
			return -1;
		}
		FD_ZERO(&rd);
		FD_SET(socketfd, &rd);
		timeout.tv_sec = Timeout/1000;
		timeout.tv_usec = Timeout%1000;
		res = select(socketfd+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			printf("Receive data reply from Server timeout\n");
			close_socket(socketfd);
			return -1;
		}else
		{
			*recvLength = recv(socketfd, recvbuff, *recvLength, 0);
			if(0 == *recvLength)
			{
				close_socket(socketfd);
				return -1;
			}else
			{
				printf("serverCommunication_Control:%s\n",recvbuff);
				close_socket(socketfd);
				return 0;
			}
		}
		
	}else
	{
		//连接服务器失败
		//失败情况下通过无线发送
#ifdef WIFI_USE
		int ret = 0,i = 0;
		ret = SendToSocketC(sendbuff, sendLength);
		if(ret == -1)
			return -1;
		for(i = 0;i<(Timeout/10);i++)
		{
			if(WIFI_Recv_SocketC_Event == 1)
			{
				*recvLength = WIFI_Recv_SocketC_LEN;
				memcpy(recvbuff,WIFI_RecvSocketCData,*recvLength);
				recvbuff[*recvLength] = '\0';
				printf("serverCommunication_Control:%s\n",recvbuff);
				WIFI_Recv_SocketC_Event = 0;
				WIFI_Close(SOCKET_C);
				return 0;
			}
			rt_hw_ms_delay(10);
		}
		WIFI_Close(SOCKET_C);
		return -1;
#endif

#ifndef WIFI_USE
		return -1;
#endif
	}
	
}




#ifdef RT_USING_FINSH
#include <finsh.h>
void testS1()
{
	char sendbuff[100] = "1234567876543456765432";
	char recvbuff[100] = {'\0'};
	int sendLength = 22;
	int recvLength = 100;
	serverCommunication_Client(sendbuff,sendLength,recvbuff,&recvLength,5000);

}
FINSH_FUNCTION_EXPORT(testS1, eg:testS1());

void testS2()
{
	char sendbuff[100] = "1234567876543456765432";
	char recvbuff[100] = {'\0'};
	int sendLength = 22;
	int recvLength = 100;
	serverCommunication_Control(sendbuff,sendLength,recvbuff,&recvLength,5000);

}
FINSH_FUNCTION_EXPORT(testS2, eg:testS2());
#endif
