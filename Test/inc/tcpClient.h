#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__
#include <rtthread.h>
#include <lwip/netdb.h> /* Ϊ�˽�������������Ҫ����netdb.hͷ�ļ� */
#include <lwip/sockets.h> /* ʹ��BSD socket����Ҫ����sockets.hͷ�ļ� */

void tcpclient(const char* url, int port);
#endif /*__TCPCLIENT_H__*/
