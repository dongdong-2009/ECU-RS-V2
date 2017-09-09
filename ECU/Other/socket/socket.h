#ifndef __SOCKET_H__
#define __SOCKET_H__

int createsocket(void);					//创建SOCKET连接
int connect_client_socket(int fd_sock);				//通过有线的方式连接服务器
int connect_control_socket(int fd_sock);				//通过有线的方式连接服务器
void close_socket(int fd_sock);					//关闭socket连接
int wifi_socketb_format(char *data ,int length);
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
#endif /*__SOCKET_H__*/
