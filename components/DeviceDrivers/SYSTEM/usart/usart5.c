/*****************************************************************************/
/* File      : usart.c                                                        */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/

#include "sys.h"
#include "usart5.h"	
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"
#include "rthw.h"
#include <rtthread.h>
#include "socket.h"
#include "stdlib.h"
#include "threadlist.h"
#include "debug.h"
#include "zigbee.h"
#include "Serverfile.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_RCC                    RCC_APB2Periph_GPIOC
#define WIFI_GPIO                   GPIOC
#define WIFI_PIN                    (GPIO_Pin_6)


rt_mutex_t wifi_uart_lock = RT_NULL;
unsigned char searchConnectNum = 0;
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int int_length(int data)
{
	int n=0;
	do {
		n++;
		data /= 10 ;
	} while( data>0 );
	return n;
}


//4个字节
unsigned short packetlen_A(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//2个字节
unsigned short packetlen_B(unsigned char *packet)
{
	unsigned short len = 0,count = 0;
	count = (packet[0] - '0') * 10 + (packet[1] - '0');
	len =  3 + 9 + count *14;
	return len;
	
}

//5个字节
unsigned short packetlen_C(unsigned char *packet)
{
	unsigned short len = 0;
	int i = 0;
	for(i = 0;i < 5;i++)
	{
		if(packet[i] == 'A') packet[i] = '0';
	}
	len = ((packet[0]-'0')*10000 +(packet[1]-'0')*1000 + (packet[2]-'0')*100 + (packet[3]-'0')*10 + (packet[4]-'0'));
	return len;
}

//WIFI发送函数 
int WIFI_SendData(char *data, int num)
{      
	int index = 0;
	char ch = 0;
	for(index = 0;index < num;index++)
	{
		ch = data[index];
		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET); 
    USART_SendData(UART5,(uint16_t)ch);
	}
	return index;
}
 

//初始化IO 串口5
//bound:波特率
void uart5_init(u32 bound){
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//使能串口时钟UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);	//使能GPIOA时钟

 	//USART_DeInit(UART5);  //复位串口5
	 //UART5_TX   PC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC12
   
    //UART5_RX	  PD.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //初始化PD2
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//收发模式

    USART_Init(UART5, &USART_InitStructure); //初始化串口

   //UART5 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(UART5, ENABLE);                    //使能串口 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	
	wifi_uart_lock = rt_mutex_create("wifi_uart_lock", RT_IPC_FLAG_FIFO);
}


//WIFI  socket A 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketAData[SOCKETA_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketA_Event = 0;
unsigned int WIFI_Recv_SocketA_LEN =0;
char TCPServerConnectID = '0';

char ConnectID = '0';
	
//WIFI  socket B 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketBData[SOCKETB_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketB_Event = 0;
unsigned int WIFI_Recv_SocketB_LEN =0;

//WIFI  socket C 当串口发出来的数据组包成功时 ,数组赋值，并且Socket事件变为1
unsigned char WIFI_RecvSocketCData[SOCKETC_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketC_Event = 0;
unsigned int WIFI_Recv_SocketC_LEN =0;


//wifi  串口当前收到的数据 
unsigned char USART_RX_BUF[USART_REC_LEN];     			//接收缓冲,最大USART_REC_LEN个字节.
unsigned short Cur = 0;															//当前采值位置														//数据解析位置

void UART5_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(UART5);//(UART5->DR);	//读取接收到的数据
		SEGGER_RTT_printf(0, "[%d] : %x %c\n",Cur,USART_RX_BUF[Cur],USART_RX_BUF[Cur]);
		Cur +=1;
		if(Cur >=USART_REC_LEN)
		{
			Cur = 0;
		}
	}
}

void clear_WIFI(void)
{
	//TIM3_Int_Deinit();
	Cur = 0;
}



//判断字符中是否有OK  字符
int detectionOK(int size)		//检测到OK  返回1   未检出到返回0
{
	int i=0;
	for(i = 0;i<(size-2);i++)
	{
		if(!memcmp(&USART_RX_BUF[i],"OK",2))
		{
			return 1;			
		}
	}
	return 0;
}

int detectionUNLINK(int size)		//检测到OK  返回1   未检出到返回0
{
	int i=0;
	for(i = 0;i<(size-2);i++)
	{
		if(!memcmp(&USART_RX_BUF[i],"UNLINK",6))
		{
			return 1;			
		}
	}
	return 0;
}


//判断字符中是否有OK  字符
int detectionSENDOK(int size)		//检测到OK  返回1   未检出到返回0
{
	int i=0;
	for(i = 0;i<(size-7);i++)
	{
		if(!memcmp(&USART_RX_BUF[i],"SEND OK",7))
		{
			return 1;			
		}
	}
	return 0;
}

//判断字符中是否有+IPD
int detectionIPD(int size)
{
	int i=0,j=0;
	char messageLen[5] = {'\0'};
	int len = 0;
	
	for(i = 0;i<(size-4);i++)
	{
		if(!memcmp(&USART_RX_BUF[i],"+IPD",4))
		{
			ConnectID = USART_RX_BUF[i+5];
			memcpy(messageLen,&USART_RX_BUF[i+7],4);
			for(j = 0;j<4;j++)
			{
				if(messageLen[j] == ':')
				{
					messageLen[j] = '\0';
					//printf("%c %d,%s\n",USART_RX_BUF[i+5],j,messageLen);
					len = atoi(messageLen);
					break;
				}
			}
			if((size - i -7-j) >=len)
			{
				if('4'==ConnectID)
				{
					memcpy(WIFI_RecvSocketCData,&USART_RX_BUF[i+8+j],len );
					WIFI_RecvSocketCData[len] = '\0';
					WIFI_Recv_SocketC_Event = 1;
					WIFI_Recv_SocketC_LEN =len;
					printf("C:%s\n",WIFI_RecvSocketCData);
				}else if('3' == ConnectID){
					memcpy(WIFI_RecvSocketBData,&USART_RX_BUF[i+8+j],len );
					WIFI_RecvSocketBData[len] = '\0';
					WIFI_Recv_SocketB_Event = 1;
					WIFI_Recv_SocketB_LEN =len;
					printf("B:%s\n",WIFI_RecvSocketBData);
				}else
				{
					TCPServerConnectID = ConnectID;
					memcpy(WIFI_RecvSocketAData,&USART_RX_BUF[i+8+j],len );
					WIFI_RecvSocketAData[len] = '\0';
					WIFI_Recv_SocketA_Event = 1;
					WIFI_Recv_SocketA_LEN =len;
					printf("A:%s\n",WIFI_RecvSocketAData);
				}
				Cur = 0;
				return 1;
			}
			break;
			
		}
	}
	return 0;
}

void WIFI_GetEvent_ESP07S(void)
{
	//判断ECU_R数据大于38个字节
	detectionIPD(Cur);
}

int WIFI_Reset(void)
{
	GPIO_ResetBits(WIFI_GPIO, WIFI_PIN);
	
	rt_hw_ms_delay(1000);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	return 0;
}

char sendbuff[4096] = {'\0'};

int ESP07S_sendData(char *data ,int length)
{
	int i = 0;
	clear_WIFI();
	WIFI_SendData(data,length);
	for(i = 0;i< 100;i++)
	{
		if(1 == detectionSENDOK(Cur))
		{
			//printf("AT+CWMODE3 :+ok\n");
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
	
}

int SendToSocket(char connectID,char *data ,int length)
{
	int send_length = 0;	//需要发送的字节位置
	while(length > 0)
	{
		memset(sendbuff,'\0',4096);
		if(length > 1460)
		{
			memcpy(sendbuff,&data[send_length],1460);
			AT_CIPSEND(connectID,1460);
			ESP07S_sendData(sendbuff,1460);
			//rt_hw_ms_delay(230);
			send_length += 1460;
			length -= 1460;
		}else
		{
			memcpy(sendbuff,&data[send_length],length);
			AT_CIPSEND(connectID,length);
			ESP07S_sendData(sendbuff,length);
			length -= length;

			return 0;
		}
		
	}
	AT_CIPCLOSE(connectID);
	return -1;
}


//SOCKET A 发送数据  \n需要在传入字符串中带入
int SendToSocketA(char *data ,int length)
{
	return SendToSocket(TCPServerConnectID,data,length);
}

//SOCKET B 发送数据
int SendToSocketB(char *IP ,int port,char *data ,int length)
{
	WIFI_Recv_SocketB_Event = 0;
	if(!AT_CIPSTART('3',"TCP",IP ,port))
	{
		//printf("SendToSocketB: ino AT_CIPSTART\n");
		return SendToSocket('3',data,length);
	}
	return -1;
}

//SOCKET C 发送数据
int SendToSocketC(char *IP ,int port,char *data ,int length)
{
	WIFI_Recv_SocketC_Event = 0;
	if(!AT_CIPSTART('4',"TCP",IP ,port))
	{
		//printf("SendToSocketC: ino AT_CIPSTART\n");
		return SendToSocket('4',data,length);
	}
	return -1;
}

//----ESP01流程-------------------------------
int AT_CWMODE3(void)			//配置WIFI模块为AP+STA模式
{
	int i = 0;
	clear_WIFI();
	WIFI_SendData("AT+CWMODE_DEF=3\r\n", 17);
	for(i = 0;i< 100;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+CWMODE3 :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
		
}

int AT_RST(void)			//复位WIFI模块
{
	int i = 0;
	clear_WIFI();
	//发送"AT+Z\n",返回+ok
	WIFI_SendData("AT+RST\r\n", 8);
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+RST :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
	
}


int WIFI_Test(void)
{
	int i = 0;
	clear_WIFI();
	//发送"AT+Z\n",返回+ok
	WIFI_SendData("AT\r\n", 8);
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT:+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int AT_CWSAP(char *ECUID,char *PASSWD)			//配置ECU热点名字
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CWSAP_DEF=\"ECU_R_%s\",\"%s\",11,3\r\n",ECUID,PASSWD);
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+CWSAP :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
	
}

int WIFI_Factory_Passwd(void)
{
	char ECUID[13] = {'\0'};
	Read_ECUID(ECUID);
	if(!AT_CWSAP(ECUID,"88888888"))
		return 0;
	else
		return -1;
}

int AT_CWJAP(char *SSID,char *PASSWD)			//配置ECU连接无线路由器名
{
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,PASSWD);
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));

	return 0;
}

int AT_CWJAP_DEF(char *SSID,char *PASSWD)			//配置ECU连接无线路由器名
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,PASSWD);
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 1500;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+AT_CWJAP_DEF :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

//判断字符中是否有OK  字符	LinksStatus:连接状态 0表示未连接 1表示已连接
int detectionJAPStatus(int size,char *info,unsigned char *LinksStatus)		//检测到OK  返回1   未检出到返回0
{
	int i=0,j=0,SSIDStart = 0,SSIDEnd = 0;
	*LinksStatus = 0;
	for(i = 0;i<(size-2);i++)
	{
		if(!memcmp(&USART_RX_BUF[i],"OK",2))
		{
			for(j = 0;j < i;j++)
			{
				if(!memcmp(&USART_RX_BUF[j],"+CWJAP:\"",8))
				{
					SSIDStart = j+7;
					for(j=SSIDStart;j < i;j++)
					{
						if('\r' == USART_RX_BUF[j])
						{
							
							SSIDEnd = j;
							break;
						}
					}
					memcpy(info,&USART_RX_BUF[SSIDStart],(SSIDEnd-SSIDStart));
					printf("info:%s\n",info);
					*LinksStatus = 1;
					return 1;
				}
			}
			*LinksStatus = 0;
			return 1;			
		}
	}
	return -1;
}

int AT_CWJAPStatus(char *info)			//查询ECU连接无线路由器名 返回1表示获取成功连接，返回0表示未连接
{
	int i = 0;
	unsigned char LinksStatus;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CWJAP?\r\n");
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionJAPStatus(Cur,info,&LinksStatus))
		{
			printf("AT+AT_CWJAPStatus :+ok:%d\n",LinksStatus);
			clear_WIFI();
			return LinksStatus;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return 0;
}

int AT_CIPMUX1(void)			//设置多连接AT命令
{
	int i = 0;
	clear_WIFI();
	WIFI_SendData("AT+CIPMUX=1\r\n", 13);
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+CIPMUX :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int AT_CIPSERVER(void)			//设置多连接AT命令
{
	int i = 0;
	clear_WIFI();
	WIFI_SendData("AT+CIPSERVER=1,8899\r\n", 21);
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+CIPSERVER=1,8899 :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}


int AT_CIPSTART(char ConnectID,char *connectType,char *IP,int port)			//配置ECU连接无线路由器名
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CIPSTART=%c,\"%s\",\"%s\",%d\r\n",ConnectID,connectType,IP,port);
	
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 500;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("%s +ok\n",AT);
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int AT_CIPCLOSE(char ConnectID)			//配置ECU连接无线路由器名
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CIPCLOSE=%c\r\n",ConnectID);
	//printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 100;i++)
	{
		if((1 == detectionOK(Cur))||(1 == detectionUNLINK(Cur)))
		{
			printf("AT+AT_CIPCLOSE :%c +ok\n",ConnectID);
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}


int AT_CIPSEND(char ConnectID,int size)			//配置ECU连接无线路由器名
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CIPSEND=%c,%d\r\n",ConnectID,size);
	//printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			//printf("AT+AT_CIPSEND :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int AT_CIPAP_DEF(void)			//配置ECU连接无线路由器名
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CIPAP_DEF=\"10.10.100.254\"\r\n");
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT+AT_CIPAP_DEF :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int AT_CIPSTO(void)			//配置WIFI模块作为TCP服务器时的超时时间
{
	int i = 0;
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CIPSTO=20\r\n");
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));
	for(i = 0;i< 200;i++)
	{
		if(1 == detectionOK(Cur))
		{
			printf("AT_CIPSTO :+ok\n");
			clear_WIFI();
			return 0;
		}
		rt_thread_delay(1);
	}
	clear_WIFI();
	return -1;
}

int WIFI_Factory(char *ECUID12)
{

	if(!AT_CWSAP(ECUID12,"88888888"))
	{

		if(0 != AT_RST())
		{
			WIFI_Reset();
		}
		rt_hw_s_delay(1);
		AT_CIPMUX1();
		AT_CIPSERVER();
		AT_CIPSTO();
		return 0;
	}
	else
		return -1;

}

int WIFI_ChangePasswd(char *NewPasswd)
{
	char ECUID[13] = {'\0'};
	Read_ECUID(ECUID);
	if(!AT_CWSAP(ECUID,NewPasswd))
		return 0;
	else
		return -1;
}

int InitWorkMode(void)
{
	int i = 0,res = 0;

	//选择 WMODE
	for(i = 0;i<3;i++)
	{
		if(0 == AT_CWMODE3())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;	

	//配置默认IP
	for(i = 0;i<3;i++)
	{
		if(0 == AT_CIPAP_DEF())
		{
			res = 0;
			break;
		}else
			res = -1;
	}
	if(res == -1) return -1;	
	
	printf("WIFI_InitWorkMode Over\n");

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void Send(char *data, int num)
{
	memset(sendbuff,0x00,4096);
	sprintf(sendbuff,"b00000000");

	memcpy(&sendbuff[9],data,num);
	
	WIFI_SendData(sendbuff,(num+9));
}
int AT_JAPST(void)
{
	char info[100] = {'\0'};

	return AT_CWJAPStatus(info);
}

FINSH_FUNCTION_EXPORT(Send ,WIFI send)


FINSH_FUNCTION_EXPORT(WIFI_Reset , Reset WIFI Module .)


FINSH_FUNCTION_EXPORT(WIFI_SendData ,WIFI_SendData)
FINSH_FUNCTION_EXPORT(AT_CWMODE3 ,AT CWMODE 3)
FINSH_FUNCTION_EXPORT(AT_RST ,AT reset)
FINSH_FUNCTION_EXPORT(AT_CWSAP ,AT_CWSAP)
FINSH_FUNCTION_EXPORT(AT_CWJAP_DEF ,AT_CWJAP_DEF)
FINSH_FUNCTION_EXPORT(AT_CIPMUX1 ,AT_CIPMUX1)
FINSH_FUNCTION_EXPORT(AT_CIPSERVER ,AT_CIPSERVER)

FINSH_FUNCTION_EXPORT(AT_CIPSTART ,AT_CIPSTART)
FINSH_FUNCTION_EXPORT(AT_CIPCLOSE ,AT_CIPCLOSE)
FINSH_FUNCTION_EXPORT(AT_CIPSEND ,AT_CIPSEND)
FINSH_FUNCTION_EXPORT(detectionIPD ,detectionIPD)

FINSH_FUNCTION_EXPORT(AT_CIPAP_DEF ,AT_CIPAP_DEF)
FINSH_FUNCTION_EXPORT(AT_CIPSTO ,AT_CIPSTO)
FINSH_FUNCTION_EXPORT(WIFI_Test ,WIFI_Test)
FINSH_FUNCTION_EXPORT(InitWorkMode ,InitWorkMode)

FINSH_FUNCTION_EXPORT(SendToSocketB , Send SOCKET B.)
FINSH_FUNCTION_EXPORT(SendToSocketC , Send SOCKET C.)


FINSH_FUNCTION_EXPORT(AT_JAPST , AT_JAPST.)
#endif

