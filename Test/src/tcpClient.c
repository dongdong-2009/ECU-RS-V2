/*
* �����嵥��TCP�ͻ�������
*/
#include "tcpclient.h"
/* �����õ������� */
ALIGN(4)
static const char send_data[] = "This is TCP Client from RT-Thread.";
void tcpclient(const char* url, int port)
{
	char *recv_data;
	struct hostent *host;
	int sock, bytes_received;
	struct sockaddr_in server_addr;
	/* ͨ��������ڲ���url���host��ַ��������������������������� */
	host = gethostbyname(url);
	/* �������ڴ�Ž������ݵĻ��� */
	recv_data = rt_malloc(1024);
	if (recv_data == RT_NULL)
	{
		rt_kprintf("No memory\n");
		return;
	}
	/* ����һ��socket��������SOCKET_STREAM��TCP���� */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		/* ����socketʧ�� */
		rt_kprintf("Socket error\n");
		/* �ͷŽ��ջ��� */
		rt_free(recv_data);
		return;
	}
	/* ��ʼ��Ԥ���ӵķ���˵�ַ */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	/* ���ӵ������ */
	if (connect(sock, (struct sockaddr *) &server_addr,
	sizeof(struct sockaddr)) == -1)
	{
		/* ����ʧ�� */
		rt_kprintf("Connect error\n");
		/*�ͷŽ��ջ��� */
		rt_free(recv_data);
		return;
	}
	while (1)
	{
		/* ��sock�����н������1024�ֽ����� */
		bytes_received = recv(sock, recv_data, 1024, 0);
		if (bytes_received < 0)
		{
			/* ����ʧ�ܣ��ر�������� */
			lwip_close(sock);
			/* �ͷŽ��ջ��� */
			rt_free(recv_data);
			break;
		}
		/* �н��յ����ݣ���ĩ������ */
		recv_data[bytes_received] = '\0';
		if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
		{
			/* ���������ĸ��q��Q���ر�������� */
			lwip_close(sock);
			/* �ͷŽ��ջ��� */
			rt_free(recv_data);
			break;
		}
		else
		{
			/* �ڿ����ն���ʾ�յ������� */
			rt_kprintf("\nRecieved data = %s ", recv_data);
		}
		/* �������ݵ�sock���� */
		send(sock, send_data, strlen(send_data), 0);
	}
	return;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
/* ���tcpclient������finsh shell�� */
FINSH_FUNCTION_EXPORT(tcpclient, startup tcp client);
#endif
