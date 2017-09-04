/*****************************************************************************/
/* File      : usart.h                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	
#include "sys.h" 

typedef enum
{ 
    EN_RECV_ST_GET_A         		= 0,		//接收数据头
    EN_RECV_ST_GET_ID          	= 1,	
    EN_RECV_ST_GET_HEAD         = 2,		//接收数据头
    EN_RECV_ST_GET_LEN          = 3,	//接收数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_DATA         = 4,	//接收数据部分数据
    EN_RECV_ST_GET_END          = 5		//接收END结尾标志
} eRecvSM;// receive state machin

typedef enum 
{
	SOCKET_A = 1,
	SOCKET_B = 2,
	SOCKET_C = 3,
} eSocketType;

#define USART_REC_LEN  			800  	//定义最大接收字节数 200
#define EN_USART5_RX 				1		//使能（1）/禁止（0）串口1接收
	  	

extern unsigned char WIFI_RecvData[USART_REC_LEN];
extern unsigned char WIFI_Recv_Event;

unsigned short packetlen(unsigned char *packet);
int WIFI_SendData(char *data, int num);
void WIFI_GetEvent(int *messageLen,unsigned char *ID);
void uart5_init(u32 bound);

int AT(void);
int AT_ENTM(void);
int AT_WAP(char *ECUID12);
int AT_WAKEY(char *NewPasswd);
int WIFI_ChangePasswd(char *NewPasswd);
int WIFI_Reset(void);
int AT_Z(void);
int WIFI_ClearPasswd(void);
int WIFI_SoftReset(void);

int WIFI_Test(void);
int WIFI_Factory(char *ECUID12);

int WIFI_Create(eSocketType Type);
int WIFI_Close(eSocketType Type);
int WIFI_QueryStatus(eSocketType Type);

int SendToSocketB(char *data ,int length);
int SendToSocketC(char *data ,int length);

#endif


