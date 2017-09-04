/*
* �����嵥��TCP���������
*/
#include <rtthread.h>
#include <lwip/sockets.h> /* ʹ��BSD Socket�ӿڱ������sockets.h���ͷ�ļ� */
/* �����õ������� */
ALIGN(4)
static const char send_data[] = "This is TCP Server from RT-Thread.";
void tcpserv(void* parameter)
{
	char *recv_data; /* ���ڽ��յ�ָ�룬�������һ�ζ�̬��������������ڴ� */
	rt_uint32_t sin_size,sum=0;
	int sock, connected, bytes_received;
	struct sockaddr_in server_addr, client_addr;
	rt_bool_t stop = RT_FALSE; /* ֹͣ��־ */
	recv_data = rt_malloc(1461); /* ��������õ����ݻ��� */
	if (recv_data == RT_NULL)
	{
		rt_kprintf("No memory\n");
		return;
	}
	
	/* һ��socket��ʹ��ǰ����ҪԤ�ȴ���������ָ��SOCK_STREAMΪTCP��socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		/* ����ʧ�ܵĴ����� */
		rt_kprintf("Socket error\n");
		/* �ͷ��ѷ���Ľ��ջ��� */
		rt_free(recv_data);
		return;
	}
	
	/* ��ʼ������˵�ַ */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000); /* ����˹����Ķ˿� */
	server_addr.sin_addr.s_addr = INADDR_ANY;
	rt_memset(&(server_addr.sin_zero), 8, sizeof(server_addr.sin_zero));
	/* ��socket������˵�ַ */
	if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr))
	== -1)
	{
		/* ��ʧ�� */
		rt_kprintf("Unable to bind\n");
		/* �ͷ��ѷ���Ľ��ջ��� */
		rt_free(recv_data);
		return;	
	}
	/* ��socket�Ͻ��м��� */
	if (listen(sock, 5) == -1)
	{
		rt_kprintf("Listen error\n");
		/* release recv buffer */
		rt_free(recv_data);
		return;
	}
	rt_kprintf("\nTCPServer Waiting for client on port 5000...\n");
	while (stop != RT_TRUE)
	{
		sin_size = sizeof(struct sockaddr_in);
		/* ����һ���ͻ�������socket�����������������������ʽ�� */
		connected = accept(sock, (struct sockaddr *) &client_addr, &sin_size);
		/* ���ص������ӳɹ���socket */
		/* ���ܷ��ص�client_addrָ���˿ͻ��˵ĵ�ַ��Ϣ */
		rt_kprintf("I got a connection from (%s , %d)\n", inet_ntoa(
		client_addr.sin_addr), ntohs(client_addr.sin_port));
		/* �ͻ������ӵĴ��� */
		while (1)
		{
			/* �������ݵ�connected socket */
			send(connected, send_data, strlen(send_data), 0);
			/*
			* ��connected socket�н������ݣ�����buffer��1024��С��
			* ������һ���ܹ��յ�1024��С������
			*/
			bytes_received = recv(connected, recv_data, 1460, 0);
			if (bytes_received < 0)
			{
				/* ����ʧ�ܣ��ر����connected socket */
				lwip_close(connected);
				break;
			}
			/* �н��յ����ݣ���ĩ������ */
			//recv_data[bytes_received] = '\0';
			if(bytes_received > 0)
			{
				sum = sum+bytes_received;
				rt_kprintf("%d   %d\n",bytes_received,sum);
				//rt_kprintf("%s", recv_data);
				rt_memset(recv_data,0x00,1461 );
			}
		#if 0	
			if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
			{
				/* ���������ĸ��q��Q���ر�������� */
				lwip_close(connected);
				break;
			}
			else if (strcmp(recv_data, "exit") == 0)
			{
				/* ������յ���exit����ر���������� */
				lwip_close(connected);
				stop = RT_TRUE;
				break;
			}
			else
			{
				/* �ڿ����ն���ʾ�յ������� */
				rt_kprintf("RECIEVED DATA = %s \n", recv_data);
			}
			#endif
		}
	}
	/* �˳����� */
	lwip_close(sock);
	/* �ͷŽ��ջ��� */
	rt_free(recv_data);
	return;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
/* ���tcpserv������finsh shell�� */
FINSH_FUNCTION_EXPORT(tcpserv, startup tcp server);
#endif
