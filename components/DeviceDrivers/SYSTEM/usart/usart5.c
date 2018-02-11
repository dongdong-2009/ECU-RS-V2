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


//4���ֽ�
unsigned short packetlen_A(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//2���ֽ�
unsigned short packetlen_B(unsigned char *packet)
{
	unsigned short len = 0,count = 0;
	count = (packet[0] - '0') * 10 + (packet[1] - '0');
	len =  3 + 9 + count *14;
	return len;
	
}

//5���ֽ�
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

//WIFI���ͺ��� 
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
 

//��ʼ��IO ����5
//bound:������
void uart5_init(u32 bound){
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ�ܴ���ʱ��UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);	//ʹ��GPIOAʱ��

 	//USART_DeInit(UART5);  //��λ����5
	 //UART5_TX   PC.12
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PC12
   
    //UART5_RX	  PD.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);  //��ʼ��PD2
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ

    USART_Init(UART5, &USART_InitStructure); //��ʼ������

   //UART5 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ��� 


    RCC_APB2PeriphClockCmd(WIFI_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = WIFI_PIN;
    GPIO_Init(WIFI_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(WIFI_GPIO, WIFI_PIN);
	
	wifi_uart_lock = rt_mutex_create("wifi_uart_lock", RT_IPC_FLAG_FIFO);
}


//WIFI  socket A �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketAData[SOCKETA_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketA_Event = 0;
unsigned int WIFI_Recv_SocketA_LEN =0;
char TCPServerConnectID = '0';

char ConnectID = '0';
	
//WIFI  socket B �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketBData[SOCKETB_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketB_Event = 0;
unsigned int WIFI_Recv_SocketB_LEN =0;

//WIFI  socket C �����ڷ���������������ɹ�ʱ ,���鸳ֵ������Socket�¼���Ϊ1
unsigned char WIFI_RecvSocketCData[SOCKETC_LEN] = {'\0'};
unsigned char WIFI_Recv_SocketC_Event = 0;
unsigned int WIFI_Recv_SocketC_LEN =0;


//wifi  ���ڵ�ǰ�յ������� 
unsigned char USART_RX_BUF[USART_REC_LEN];     			//���ջ���,���USART_REC_LEN���ֽ�.
unsigned short Cur = 0;															//��ǰ��ֵλ��														//���ݽ���λ��

void UART5_IRQHandler(void)                	//����1�жϷ������
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		USART_RX_BUF[Cur] = USART_ReceiveData(UART5);//(UART5->DR);	//��ȡ���յ�������
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



//�ж��ַ����Ƿ���OK  �ַ�
int detectionOK(int size)		//��⵽OK  ����1   δ���������0
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

int detectionUNLINK(int size)		//��⵽OK  ����1   δ���������0
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


//�ж��ַ����Ƿ���OK  �ַ�
int detectionSENDOK(int size)		//��⵽OK  ����1   δ���������0
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

//�ж��ַ����Ƿ���+IPD
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
	//�ж�ECU_R���ݴ���38���ֽ�
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
	int send_length = 0;	//��Ҫ���͵��ֽ�λ��
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


//SOCKET A ��������  \n��Ҫ�ڴ����ַ����д���
int SendToSocketA(char *data ,int length)
{
	return SendToSocket(TCPServerConnectID,data,length);
}

//SOCKET B ��������
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

//SOCKET C ��������
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

//----ESP01����-------------------------------
int AT_CWMODE3(void)			//����WIFIģ��ΪAP+STAģʽ
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

int AT_RST(void)			//��λWIFIģ��
{
	int i = 0;
	clear_WIFI();
	//����"AT+Z\n",����+ok
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
	//����"AT+Z\n",����+ok
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

int AT_CWSAP(char *ECUID,char *PASSWD)			//����ECU�ȵ�����
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

int AT_CWJAP(char *SSID,char *PASSWD)			//����ECU��������·������
{
	char AT[100] = { '\0' };
	clear_WIFI();
	sprintf(AT,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n",SSID,PASSWD);
	printf("%s",AT);
	WIFI_SendData(AT, (strlen(AT)+1));

	return 0;
}

int AT_CWJAP_DEF(char *SSID,char *PASSWD)			//����ECU��������·������
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

//�ж��ַ����Ƿ���OK  �ַ�	LinksStatus:����״̬ 0��ʾδ���� 1��ʾ������
int detectionJAPStatus(int size,char *info,unsigned char *LinksStatus)		//��⵽OK  ����1   δ���������0
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

int AT_CWJAPStatus(char *info)			//��ѯECU��������·������ ����1��ʾ��ȡ�ɹ����ӣ�����0��ʾδ����
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

int AT_CIPMUX1(void)			//���ö�����AT����
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

int AT_CIPSERVER(void)			//���ö�����AT����
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


int AT_CIPSTART(char ConnectID,char *connectType,char *IP,int port)			//����ECU��������·������
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

int AT_CIPCLOSE(char ConnectID)			//����ECU��������·������
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


int AT_CIPSEND(char ConnectID,int size)			//����ECU��������·������
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

int AT_CIPAP_DEF(void)			//����ECU��������·������
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

int AT_CIPSTO(void)			//����WIFIģ����ΪTCP������ʱ�ĳ�ʱʱ��
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

	//ѡ�� WMODE
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

	//����Ĭ��IP
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

