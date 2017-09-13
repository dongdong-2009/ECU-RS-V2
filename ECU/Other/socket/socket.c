#include "socket.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "lan8720rst.h"
#include <stdio.h>
#include "rtc.h"
#include "threadlist.h"
#include "usart5.h"
#include "debug.h"
#include "string.h"
#include "rthw.h"

#define CLIENT_SERVER_DOMAIN	""
#define CLIENT_SERVER_IP			"60.190.131.190"
//#define CLIENT_SERVER_IP			"192.168.1.102"
#define CLIENT_SERVER_PORT1	8982
#define CLIENT_SERVER_PORT2	8982

#define CONTROL_SERVER_DOMAIN	""
//#define CONTROL_SERVER_IP			"139.168.200.158"
#define CONTROL_SERVER_IP			"139.168.200.158"
#define CONTROL_SERVER_PORT1	8997
#define CONTROL_SERVER_PORT2	8997


//初始化USR锁
void initUSRLock(void)
{

}

int createsocket(void)					//创建SOCKET连接
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg(ECU_DBG_OTHER,"Failed to create socket");
	else
		printmsg(ECU_DBG_OTHER,"Create socket successfully");

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
		printmsg(ECU_DBG_CLIENT,"Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		sprintf(ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
	}

	//printf("IP:%s\n", ip);
	//printf("Port1:%d\n", port[0]);
	//printf("Port2:%d\n", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[0]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		printmsg(ECU_DBG_CLIENT,"Failed to connect to EMA");
		closesocket(fd_sock);
		return -1;
	}
	else{
		printmsg(ECU_DBG_CLIENT,"Connect to EMA Client successfully");
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
		printmsg(ECU_DBG_CONTROL_CLIENT,"Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		sprintf(ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
	}

	//printf("IP:%s\n", ip);
	//printf("Port1:%d\n", port[0]);
	//printf("Port2:%d\n", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[0]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Failed to connect to EMA");
		closesocket(fd_sock);
		return -1;
	}
	else{
		printmsg(ECU_DBG_CONTROL_CLIENT,"Connect to EMA Client successfully");
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭socket连接
{
	closesocket(fd_sock);
	printmsg(ECU_DBG_OTHER,"Close socket");
}

int msg_is_complete(const char *s)
{
	int i, msg_length = 18;
	char buffer[6] = {'\0'};

	if(strlen(s) < 10)
		return 0;

	//从信息头获取信息长度
	strncpy(buffer, &s[5], 5);
	for(i=0; i<5; i++)
		if('A' == buffer[i])
			buffer[i] = '0';
	msg_length = atoi(buffer);

	//将实际收到的长度与信息中定义的长度作比较
	if(strlen(s) < msg_length){
		return 0;
	}
	return 1;
}



/* Socket 发送数据 */
int send_socket(int sockfd, char *sendbuffer, int size)
{
	int i, send_count;
	char msg_length[6] = {'\0'};

	if(sendbuffer[strlen(sendbuffer)-1] == '\n'){
		sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	}
	else{
		sprintf(msg_length, "%05d", strlen(sendbuffer));
		strcat(sendbuffer, "\n");
		size++;
	}
	strncpy(&sendbuffer[5], msg_length, 5);
	for(i=0; i<3; i++){
		send_count = send(sockfd, sendbuffer, size, 0);
		if(send_count >= 0){
			sendbuffer[strlen(sendbuffer)-1] = '\0';
			print2msg(ECU_DBG_CONTROL_CLIENT,"Sent", sendbuffer);
			rt_hw_ms_delay(100);
			return send_count;
		}
	}
	printmsg(ECU_DBG_CONTROL_CLIENT,"Send failed:");
	return -1;
}

/* 接收数据 */
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s)
{
	fd_set rd;
	struct timeval timeout = {10,0};
	int recv_each = 0, recv_count = 0,res = 0;
	char *recv_buffer = NULL;
	recv_buffer = malloc(4096);

	memset(recvbuffer, '\0', size);
	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(sockfd, &rd);
		timeout.tv_sec = timeout_s;
		res = select(sockfd+1, &rd, NULL, NULL, &timeout);
		switch(res){
			case -1:
				printmsg(ECU_DBG_CONTROL_CLIENT,"select");
			case 0:
				printmsg(ECU_DBG_CONTROL_CLIENT,"Receive date from EMA timeout");
				closesocket(sockfd);
				printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
				free(recv_buffer);
				recv_buffer = NULL;
				return -1;
			default:
				if(FD_ISSET(sockfd, &rd)){
					memset(recv_buffer, '\0', sizeof(recv_buffer));
					recv_each = recv(sockfd, recv_buffer, 4096, 0);
					recv_buffer[recv_each] = '\0';
					strcat(recvbuffer, recv_buffer);
					if(recv_each <= 0){	
						printdecmsg(ECU_DBG_CONTROL_CLIENT,"Communication over", recv_each);
						free(recv_buffer);
						recv_buffer = NULL;
						return -1;
					}
					printdecmsg(ECU_DBG_CONTROL_CLIENT,"Received each time", recv_each);
					recv_count += recv_each;
					print2msg(ECU_DBG_CONTROL_CLIENT,"Received", recvbuffer);
					if(msg_is_complete(recvbuffer)){
						free(recv_buffer);
						recv_buffer = NULL;
						return recv_count;
					}
				}
				break;
		}
	}

}


/* Socket客户端初始化 返回-1表示失败*/ 
int Control_client_socket_init(void)
{
	int sockfd;
	int ret;

	sockfd = createsocket();
	//创建SOCKET描述符
	if(sockfd < 0) return sockfd;
	ret = connect_control_socket(sockfd);
	//小于0  表示创建失败
	if(ret < 0) return ret;
	return sockfd;
}


int wifi_socketb_format(char *data ,int length)
{
	char head[9] = {'\0'};
	char *p = NULL;
	int i = 0,retlength = 0;

	head[0] = 'b';
	head[1] = 0x00;
	head[2] = 0x00;
	head[3] = 0x00;
	head[4] = 0x00;
	head[5] = 0x00;
	head[6] = 0x00;
	head[7] = 0x00;
	head[8] = 0x00;
	/*
	head[0] = 'b';
	head[1] = '0';
	head[2] = '0';
	head[3] = '0';
	head[4] = '0';
	head[5] = '0';
	head[6] = '0';
	head[7] = '0';
	head[8] = '0';
	*/
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
	int readbytes = 0,count =0,recvlen =0;
	char *readbuff = NULL;
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
		readbuff = malloc((4+99*14));
		memset(readbuff,'\0',(4+99*14));
		
		while(1)
		{
			FD_ZERO(&rd);
			FD_SET(socketfd, &rd);
			timeout.tv_sec = Timeout/1000;
			timeout.tv_usec = Timeout%1000;
			
			res = select(socketfd+1, &rd, NULL, NULL, &timeout);
			
			if(res <= 0){
				printmsg(ECU_DBG_CLIENT,"Receive data reply from Server timeout");
				close_socket(socketfd);
				free(readbuff);
				readbuff = NULL;
				return -1;
			}else
			{
				memset(readbuff, '\0', sizeof(readbuff));
				readbytes = recv(socketfd, readbuff, *recvLength, 0);
				if(0 == readbytes)
				{
					free(readbuff);
					readbuff = NULL;
					*recvLength = 0;
					close_socket(socketfd);
					return -1;
				}	
				strcat(recvbuff,readbuff);
				recvlen += readbytes;
				if(recvlen >= 3)
				{
					print2msg(ECU_DBG_CLIENT,"recvbuff:",recvbuff);
					*recvLength = recvlen;
					count = (recvbuff[1]-0x30)*10 + (recvbuff[2]-0x30);
					if(count==((strlen(recvbuff)-3)/14))
					{
						free(readbuff);
						readbuff = NULL;
						close_socket(socketfd);
						return *recvLength;
					}		
				}

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
					print2msg(ECU_DBG_CLIENT,"serverCommunication_Client",recvbuff);
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
			printmsg(ECU_DBG_CONTROL_CLIENT,"Receive data reply from Server timeout");
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
				print2msg(ECU_DBG_CONTROL_CLIENT,"serverCommunication_Control:",recvbuff);
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
				print2msg(ECU_DBG_CONTROL_CLIENT,"serverCommunication_Control:",recvbuff);
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
