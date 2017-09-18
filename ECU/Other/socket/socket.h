#ifndef __SOCKET_H__
#define __SOCKET_H__

#if 1
#define CLIENT_SERVER_DOMAIN	""
#define CLIENT_SERVER_IP			"60.190.131.190"
#define CLIENT_SERVER_PORT1	8982
#define CLIENT_SERVER_PORT2	8982

#define CONTROL_SERVER_DOMAIN	""
#define CONTROL_SERVER_IP		"139.168.200.158"
#define CONTROL_SERVER_PORT1	8981
#define CONTROL_SERVER_PORT2	8981

#else

#define CLIENT_SERVER_DOMAIN	""
#define CLIENT_SERVER_IP			"139.168.200.158"
//#define CLIENT_SERVER_IP			"192.168.1.110"

#define CLIENT_SERVER_PORT1	8096
#define CLIENT_SERVER_PORT2	8096

#define CONTROL_SERVER_DOMAIN	""
//#define CONTROL_SERVER_IP			"139.168.200.158"
#define CONTROL_SERVER_IP			"192.168.1.110"

#define CONTROL_SERVER_PORT1	8981
#define CONTROL_SERVER_PORT2	8981

#endif


void initUSRLock(void);
int createsocket(void);					//创建SOCKET连接
int connect_client_socket(int fd_sock);				//通过有线的方式连接服务器
int connect_control_socket(int fd_sock);				//通过有线的方式连接服务器
void close_socket(int fd_sock);					//关闭socket连接
int msg_is_complete(const char *s);
int send_socket(int sockfd, char *sendbuffer, int size);
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s);
int recv_response(int fd_sock, char *readbuff);
int Control_client_socket_init(void);
int wifi_socketb_format(char *data ,int length);
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
#endif /*__SOCKET_H__*/
