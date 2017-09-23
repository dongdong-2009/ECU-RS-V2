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
    EN_RECV_ST_GET_SCOKET_HEAD 	= 0,	//接收Socket数据头
    EN_RECV_ST_GET_SCOKET_ID    = 1,	//接收手机Socket ID
    EN_RECV_ST_GET_A_HEAD      	= 2,	//接收报文数据头
    EN_RECV_ST_GET_A_LEN        = 3,	//接收报文数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_A_DATA       = 4,	//接收报文数据部分数据
    EN_RECV_ST_GET_A_END        = 5,	//接收报文END结尾标志
	
		EN_RECV_ST_GET_B_HEAD       = 6,
		EN_RECV_ST_GET_B_LEN        = 7,
		EN_RECV_ST_GET_B_DATA       = 8,
		EN_RECV_ST_GET_B_END        = 9,
	
		EN_RECV_ST_GET_C_HEAD      	= 10,	//接收报文数据头
    EN_RECV_ST_GET_C_LEN        = 11,	//接收报文数据长度   其中数据部分的长度为接收到长度减去12个字节
    EN_RECV_ST_GET_C_DATA       = 12,	//接收报文数据部分数据
    EN_RECV_ST_GET_C_END        = 13,	//接收报文END结尾标志
	
} eRecvSM;// receive state machin

typedef enum 
{
	EN_RECV_TYPE_UNKNOWN	= 0,				//未知数据包
	EN_RECV_TYPE_A    		= 1,				//采集的是SOCKET A的数据
	EN_RECV_TYPE_B    		= 2,				//采集的是SOCKET B的数据
	EN_RECV_TYPE_C    		= 3,				//采集的是SOCKET C的数据
} eRecvType;

typedef enum 
{
	SOCKET_A = 1,
	SOCKET_B = 2,
	SOCKET_C = 3,
} eSocketType;

#define USART_REC_LEN  				2048  	//定义最大接收字节数 2048
#define SOCKETA_LEN						2048
#define SOCKETB_LEN						1408		
#define SOCKETC_LEN						2048

extern unsigned char WIFI_RecvSocketAData[SOCKETA_LEN];
extern unsigned char WIFI_Recv_SocketA_Event;
extern unsigned int WIFI_Recv_SocketA_LEN;
extern unsigned char ID_A[9];

extern unsigned char WIFI_RecvSocketBData[SOCKETB_LEN];
extern unsigned char WIFI_Recv_SocketB_Event;
extern unsigned int WIFI_Recv_SocketB_LEN;

extern unsigned char WIFI_RecvSocketCData[SOCKETC_LEN];
extern unsigned char WIFI_Recv_SocketC_Event;
extern unsigned int WIFI_Recv_SocketC_LEN;
unsigned short packetlen_A(unsigned char *packet);
unsigned short packetlen_B(unsigned char *packet);
unsigned short packetlen_C(unsigned char *packet);
	  	

int WIFI_SendData(char *data, int num);
void WIFI_GetEvent(void);
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

int SendToSocketA(char *data ,int length,unsigned char ID[8]);
int SendToSocketB(char *data ,int length);
int SendToSocketC(char *data ,int length);

#endif


