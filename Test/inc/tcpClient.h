#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__
#include <rtthread.h>
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */

void tcpclient(const char* url, int port);
#endif /*__TCPCLIENT_H__*/
