#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "threadlist.h"

typedef struct socket_config
{
	char domain[32];	//����
	char ip[16];		//IP
	int port1;			//�˿�1
	int port2;			//�˿�2

}Socket_Cfg;

extern Socket_Cfg client_arg;
extern Socket_Cfg control_client_arg;

void initSocketArgs(void);
int randport(Socket_Cfg cfg);
int createsocket(void);					//����SOCKET����
int connect_client_socket(int fd_sock);				//ͨ�����ߵķ�ʽ���ӷ�����
int connect_control_socket(int fd_sock);				//ͨ�����ߵķ�ʽ���ӷ�����
void close_socket(int fd_sock);					//�ر�socket����
int msg_is_complete(const char *s);
int send_socket(int sockfd, char *sendbuffer, int size);
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s);
int recv_response(int fd_sock, char *readbuff);
int Control_client_socket_init(void);
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
#endif /*__SOCKET_H__*/
