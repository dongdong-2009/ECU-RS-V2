#ifndef __SOCKET_H__
#define __SOCKET_H__

int createsocket(void);					//����SOCKET����
int connect_client_socket(int fd_sock);				//ͨ�����ߵķ�ʽ���ӷ�����
int connect_control_socket(int fd_sock);				//ͨ�����ߵķ�ʽ���ӷ�����
void close_socket(int fd_sock);					//�ر�socket����
int wifi_socketb_format(char *data ,int length);
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout);
#endif /*__SOCKET_H__*/
