#include "TrinaSolar.h"
#include "socket.h"
#include "dfs_posix.h"
#include "threadlist.h"
#include "debug.h"
#include "rtthread.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "variation.h"
#include "modbus.h"
#include "rtc.h"
#include "version.h"

#define TRINA_MAJOR_VERSION 0x01
#define TRINA_MINOR_VERSION 0x00
#define COMPANY_CODE    "TT"

extern ecu_info ecu;	//ecu�����Ϣ
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
unsigned char TrinaSolarDataIndex = 1;
rt_mutex_t TrinaSend_lock = RT_NULL;        //sockect������
Socket_Cfg TrinaSolar_arg;  //������������Ϣ
extern rt_mutex_t record_data_lock;

int TrinaSolarSocketFD = -1;		//���SOCKET�ļ�������
unsigned char TrinaSolarConnectFlag = 0;	//����������ӶϿ���־
unsigned char TrinaSolarHeartbeatFlag = 0;  //1��ʾ���յ�������
unsigned char TrinaSolarFunctionFlag = 0;
unsigned char TrinaRecvBuff[4096];
int TrinaRecvSize = 0;

#ifdef THREAD_PRIORITY_TRINASOLAR_RECV
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t recv_stack[512];
static struct rt_thread recv_thread;
#endif

#ifdef THREAD_PRIORITY_TRINASOLAR_HEARTBATE
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t heartbeat_stack[512];
static struct rt_thread heartbeat_thread;
#endif


void init_TrinaSendLock(void)      //��ʼ��TrinaSolar������
{
    if(TrinaSend_lock != RT_NULL)
    {
        rt_mutex_delete (TrinaSend_lock);
    }

    TrinaSend_lock = rt_mutex_create("Trina_lock", RT_IPC_FLAG_FIFO);
    if (TrinaSend_lock != RT_NULL)
    {

        printf("Initialize Trina_lock successful!\n");
    }
    
}


void initTrinaSolarSocketArgs(void)
{
    FILE *fp;
    char *buff = NULL;
    TrinaSolar_arg.port1 = TRINASOLAR_SERVER_Port;
    TrinaSolar_arg.port2 = TRINASOLAR_SERVER_Port;
    strcpy(TrinaSolar_arg.domain, TRINASOLAR_SERVER_DOMAIN);
    strcpy(TrinaSolar_arg.ip, TRINASOLAR_SERVER_IP);

    buff = malloc(512);
    memset(buff,'\0',512);
    //��ʼ�������ϴ�����
    fp = fopen("/config/trina.con", "r");
    if(fp)
    {
        while(1)
        {
            memset(buff, '\0', 512);
            fgets(buff, 512, fp);

            if(!strlen(buff))
                break;
            if(!strncmp(buff, "Domain", 6))
            {
                strcpy(TrinaSolar_arg.domain, &buff[7]);
                if('\n' == TrinaSolar_arg.domain[strlen(TrinaSolar_arg.domain)-1])
                    TrinaSolar_arg.domain[strlen(TrinaSolar_arg.domain)-1] = '\0';
            }
            if(!strncmp(buff, "IP", 2))
            {
                strcpy(TrinaSolar_arg.ip, &buff[3]);
                if('\n' == TrinaSolar_arg.ip[strlen(TrinaSolar_arg.ip)-1])
                    TrinaSolar_arg.ip[strlen(TrinaSolar_arg.ip)-1] = '\0';
            }
            if(!strncmp(buff, "Port", 5))
            {
                TrinaSolar_arg.port1=atoi(&buff[6]);
                TrinaSolar_arg.port2=atoi(&buff[6]);
            }

        }
        fclose(fp);
    }

    free(buff);
    buff = NULL;
    printf("TrinaSolar_arg.domain:%s\n",TrinaSolar_arg.domain);
    printf("TrinaSolar_arg.ip:%s\n",TrinaSolar_arg.ip);
    printf("TrinaSolar_arg.port1:%d\n",TrinaSolar_arg.port1);
    printf("TrinaSolar_arg.port2:%d\n",TrinaSolar_arg.port2);
}

void save_TrinaSolar_Record(char *Time,char *buff,int length)
{
    char dir[50] = "/home/record/tridata/";
    char file[14] = {'\0'};
    rt_err_t result;
    int fd;

    memcpy(file,&Time[2],8);
    file[8] = '.';
    memcpy(&file[9],&Time[10],3);
    file[12] = '\0';
    sprintf(dir,"%s%s",dir,file);
    printf("save_TrinaSolar_record DIR:%s\n",dir);
    result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
    if(result == RT_EOK)
    {
        fd = open(dir, O_WRONLY | O_CREAT | O_TRUNC,0);
        if (fd >= 0)
        {
            write(fd,buff,length);
            close(fd);
        }
    }
    rt_mutex_release(record_data_lock);
}
void Collect_TrinaSolar_Record(void)
{
    char *Trina_Data = NULL;
    int packlength = 0;
    int energy = 0;
    unsigned short CRC = 0;
    inverter_info *curinverter = inverterInfo;
    int i = 0;
    if(ecu.validNum > 0)
    {
        Trina_Data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
        memset(Trina_Data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
        Trina_Data[0] = 'T';
        Trina_Data[1] = 'S';
        memcpy(&Trina_Data[2],ecu.ECUID12,12);
        memcpy(&Trina_Data[27],ecu.ECUID12,12);
        Trina_Data[52] = 0x02;
        Trina_Data[53] = 0x01;
        Trina_Data[54] = TRINA_MAJOR_VERSION;
        Trina_Data[55] = TRINA_MINOR_VERSION;
        Trina_Data[56] = 0;
        Trina_Data[57] = 0;

        //������
        //���������		����2λС��
        Trina_Data[58] = ((unsigned int)(ecu.life_energy*100)/16777216)%256;
        Trina_Data[59] = ((unsigned int)(ecu.life_energy*100)/65536)%256;
        Trina_Data[60] = ((unsigned int)(ecu.life_energy*100)/256)%256;
        Trina_Data[61] =  (unsigned int)(ecu.life_energy*100)%256;
        //�������
        Trina_Data[62] = ecu.validNum;

        //�����
        Trina_Data[63] = ecu.count;
        //�ϱ�ʱ��
        Trina_Data[64] = (ecu.curTime[0] - '0')*16+(ecu.curTime[1] - '0');
        Trina_Data[65] = (ecu.curTime[2] - '0')*16+(ecu.curTime[3] - '0');
        Trina_Data[66] = (ecu.curTime[4] - '0')*16+(ecu.curTime[5] - '0');
        Trina_Data[67] = (ecu.curTime[6] - '0')*16+(ecu.curTime[7] - '0');
        Trina_Data[68] = (ecu.curTime[8] - '0')*16+(ecu.curTime[9] - '0');
        Trina_Data[69] = (ecu.curTime[10] - '0')*16+(ecu.curTime[11] - '0');
        Trina_Data[70] = (ecu.curTime[12] - '0')*16+(ecu.curTime[13] - '0');
        //����
        Trina_Data[71] = TrinaSolarDataIndex++;
        packlength = 72;
        for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)
        {
            //�ж��Ƿ�ͨѶ�ϣ��һ���Ϊjbox

            if((1 == curinverter->status.comm_failed3_status)&& (1 == curinverter->model))
            {
                //������
                memcpy(&Trina_Data[packlength],curinverter->uid,12);
                packlength += 15;
                //�豸����
                Trina_Data[packlength++] = 3;
                //�����ѹ	����1λС��
                Trina_Data[packlength++] = curinverter->PV1/256;
                Trina_Data[packlength++] = curinverter->PV1%256;
                //���빦��	����1λС��
                Trina_Data[packlength++] = (int)curinverter->AveragePower1/256;
                Trina_Data[packlength++] = (int)curinverter->AveragePower1%256;
                //�������	����1λС��
                Trina_Data[packlength++] = curinverter->PI_Output/256;
                Trina_Data[packlength++] = curinverter->PI_Output%256;
                //�¶�
                Trina_Data[packlength++] = curinverter->temperature - 100;
                //�������	����6λС��
                energy = curinverter->EnergyPV1 / (3600000/1000000);
                Trina_Data[packlength++] = (energy/65536)%256;
                Trina_Data[packlength++] = (energy/256)%256;
                Trina_Data[packlength++] = energy%256;
                //RSDʹ��״̬
                Trina_Data[packlength++] = curinverter->status.function_status;
            }
            curinverter++;

        }
        
        Trina_Data[56] = (packlength - 58)/256;
        Trina_Data[57] = (packlength - 58)%256;
        CRC = computeCRC((unsigned char*)Trina_Data, packlength);

        Trina_Data[packlength++] = (CRC >> 8);
        Trina_Data[packlength++] = (CRC & 0xFF);

				printf("%d\n",packlength);
        for( i =0;i<packlength;i++)
        {
            printf("%02x ",Trina_Data[i]);
        }
        printf("\n");
	if(ecu.count > 0)
	{
		save_TrinaSolar_Record(ecu.curTime,Trina_Data,packlength);
	}
	
        free(Trina_Data);
        Trina_Data = NULL;
    }
}

int TrinaFunction(void)	//�鿴��Ϸ����������Ƿ��
{
    int fd;
    char buff[2] = {'\0'};

    fd = open("/CONFIG/TRINAFLG.CON", O_RDONLY, 0);
    if(fd >= 0)
    {
        memset(buff, '\0', sizeof(buff));
        read(fd, buff, 1);
        close(fd);
        if(buff[0] == '1')
        {
            TrinaSolarFunctionFlag = 1;
            return 1;
        }
    }
    TrinaSolarFunctionFlag = 0;
    return -1;
}

//����һ��socket���ӷ�����������
int Trina_socket_connect(char *domain,char *IP,int port)
{
    char IPaddr[20];
    struct sockaddr_in serv_addr;
    struct hostent * host;
    TrinaSolarSocketFD = socket(AF_INET,SOCK_STREAM,0);  //����SOCKET
    if(-1 == TrinaSolarSocketFD)
    {
        printf("Trina_socket_connect Failed to create socket\n");
        return -1;
    }

    host = gethostbyname(domain);
    if(NULL == host)
    {
        printf("Trina_socket_connect Resolve domain failure\n");
        strcpy(IPaddr,IP);
    }
    else
    {
        memset(IPaddr, '\0', sizeof(IP));
        sprintf(IPaddr,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
    }

    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);
    serv_addr.sin_addr.s_addr=inet_addr(IPaddr);
    memset(&(serv_addr.sin_zero),0,8);
    //����socket
    if(-1==connect(TrinaSolarSocketFD,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
        printmsg(ECU_DBG_OTHER,"Trina_socket_connect Failed to connect to Trina");
        closesocket(TrinaSolarSocketFD);
        TrinaSolarSocketFD = -1;
        return -1;
    }
    else{
        printf("TrinaSolarSocketFD:%d\n",TrinaSolarSocketFD);
        return TrinaSolarSocketFD;
    }

}

//��������
int Trina_SendData(int sockfd, char *sendbuffer, int size)
{
    int send_count = 0;
    rt_err_t result;
    if(sockfd < 0)
    {
        TrinaSolarConnectFlag = 0;  //��ʾ��ǰ�����Ѿ��Ͽ�
        return -1;
    }
    //������
    result = rt_mutex_take(TrinaSend_lock, RT_WAITING_FOREVER);
    if(result == RT_EOK)
    {
        send_count = send(sockfd, sendbuffer, size, 0);
        printf("send_count:%d\n",send_count);
        if(send_count < 0)  //��������С��0,��ʾ��ǰ״̬�������δ����
        {
            TrinaSolarConnectFlag = 0;  //��ʾ��ǰ�����Ѿ��Ͽ�
            rt_mutex_release(TrinaSend_lock);
            return -1;
        }
    }
    rt_mutex_release(TrinaSend_lock);
    return send_count;
}
//��������
int Trina_RecvData(int sockfd, char *recvbuffer, int *size, int timeout_s)
{
    fd_set rd;
    struct timeval timeout = {10,0};
    int recv_count = 0,res;

    FD_ZERO(&rd);
    FD_SET(sockfd, &rd);
    timeout.tv_sec = timeout_s;
    res = select(sockfd+1, &rd, NULL, NULL, &timeout);
    switch(res){
    case -1:
        printf("Trina_RecvData select failed\n");
        TrinaSolarConnectFlag = 0;
        return -2;
    case 0:
        //printf("Trina_RecvData Receive date timeout\n");
        return -1;
    default:
        if(FD_ISSET(sockfd, &rd)){
            memset(recvbuffer, '\0', sizeof(recvbuffer));
            recv_count = recv(sockfd, recvbuffer, 4096, 0);
            recvbuffer[recv_count] = '\0';
            *size = recv_count;
            printf("Trina_RecvData:%d  %s\n", *size,recvbuffer);

            return recv_count;
        }
    }
    return 0;
}

int Trina_Login(void)   //��¼
{
    int ret = 0;
    char sendbuff[128] = { 0x00 };
    unsigned short CRC = 0,CRC_calc = 0,CRC_read = 0,CRClen = 0;
    char curTime[15] = {'\0'};

    memset(TrinaRecvBuff,0x00,4096);
    ret = Trina_socket_connect(TrinaSolar_arg.domain,TrinaSolar_arg.ip,TrinaSolar_arg.port1);
    if(ret >= 0)
    {
        apstime(curTime);
        curTime[14] = '\0';

        sendbuff[0] = 'T';
        sendbuff[1] = 'S';
        memcpy(&sendbuff[2],ecu.ECUID12,12);
        memcpy(&sendbuff[27],ecu.ECUID12,12);
        sendbuff[52] = 0x00;
        sendbuff[53] = 0x01;
        sendbuff[54] = TRINA_MAJOR_VERSION;
        sendbuff[55] = TRINA_MINOR_VERSION;
        sendbuff[56] = 0;
        sendbuff[57] = 53;
        //����        2�ֽ� 58
        sendbuff[58] = COMPANY_CODE[0];
        sendbuff[59] = COMPANY_CODE[1];
        //�ͺ�       13�ֽ�    60
        memcpy(&sendbuff[60],ECU_VERSION,strlen(ECU_VERSION));

        //CCID       20�ֽ�   73

        //�豸ʱ��    7�ֽ�    93
        sendbuff[93] = (curTime[0] - '0')*16+(curTime[1] - '0');
        sendbuff[94] = (curTime[2] - '0')*16+(curTime[3] - '0');
        sendbuff[95] = (curTime[4] - '0')*16+(curTime[5] - '0');
        sendbuff[96] = (curTime[6] - '0')*16+(curTime[7] - '0');
        sendbuff[97] = (curTime[8] - '0')*16+(curTime[9] - '0');
        sendbuff[98] = (curTime[10] - '0')*16+(curTime[11] - '0');
        sendbuff[99] = (curTime[12] - '0')*16+(curTime[13] - '0');

        //�̼��汾    10�ֽ�    100
        sprintf(&sendbuff[100],"%s.%s",MAJORVERSION,MINORVERSION);
        //���ұ�׼    1�ֽ�   110
        sendbuff[110] =0x01;
        CRC = computeCRC((unsigned char*)sendbuff, 111);

        sendbuff[111] = (CRC >> 8);
        sendbuff[112] = (CRC & 0xFF);
        if(Trina_SendData(TrinaSolarSocketFD, sendbuff, 113) >= 0)
        {
            if(Trina_RecvData(TrinaSolarSocketFD, (char*)TrinaRecvBuff, &TrinaRecvSize, 10) > 0)
            {
                CRClen = TrinaRecvSize-2;
                CRC_calc = computeCRC(TrinaRecvBuff, CRClen);
                CRC_read = ((((unsigned short)TrinaRecvBuff[CRClen] << 8) & 0xff00)
                            | ((unsigned short)TrinaRecvBuff[CRClen +1] & 0x00ff));
                if(CRC_calc == CRC_read)
                {
                    TrinaSolarConnectFlag = 1;
                    printf("Trina_Login successful\n");
                    return 0;
                }else
                {
                    return -4; //��������У��ʧ��
                }
            }else
            {
                return -3;  //��������ʧ��
            }

        }else
        {
            return -2;  //��������ʧ��
        }

    }else
    {
        return -1;      //���ӷ�����ʧ��
    }
}

int TrinaRecvEvent(void)
{
    unsigned short CRC_calc = 0,CRC_read = 0,CRClen = 0;
    if(Trina_RecvData(TrinaSolarSocketFD, (char*)TrinaRecvBuff, &TrinaRecvSize, 10) > 0)
    {
        CRClen = TrinaRecvSize-2;
        CRC_calc = computeCRC(TrinaRecvBuff, CRClen);
        CRC_read = ((((unsigned short)TrinaRecvBuff[CRClen] << 8) & 0xff00)
                    | ((unsigned short)TrinaRecvBuff[CRClen +1] & 0x00ff));
        if(CRC_calc == CRC_read)
        {
            return 1;
        }else
        {
            return -2; //��������У��ʧ��
        }
    }else
    {
        return -1;  //��������ʧ��
    }
}

void process_TrinaEvent(int Data_Len,const unsigned char *recvbuffer)
{
    unsigned short TrinaCMD = 0;

    TrinaCMD = recvbuffer[52] * 256 + recvbuffer[53];
    printf("TrinaCMD:%x\n",TrinaCMD);
    if(TrinaCMD == 0x0801)
    {

    }else if(TrinaCMD == 0x0802)
    {
        TrinaSolarHeartbeatFlag = 1;
    }else
    {
        ;
    }

}

int Trina_Heartbeat(void)
{
    int index = 0;
    char sendbuff[128] = { 0x00 };
    unsigned short CRC = 0;

    sendbuff[0] = 'T';
    sendbuff[1] = 'S';
    memcpy(&sendbuff[2],ecu.ECUID12,12);
    memcpy(&sendbuff[27],ecu.ECUID12,12);
    sendbuff[52] = 0x00;
    sendbuff[53] = 0x02;
    sendbuff[54] = TRINA_MAJOR_VERSION;
    sendbuff[55] = TRINA_MINOR_VERSION;
    sendbuff[56] = 0;
    sendbuff[57] = 0;

    CRC = computeCRC((unsigned char*)sendbuff, 58);

    sendbuff[58] = (CRC >> 8);
    sendbuff[59] = (CRC & 0xFF);
    TrinaSolarHeartbeatFlag = 0;
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, 60) >= 0)
    {
        for(index = 0;index<3;index++)
        {
            if(1 == TrinaSolarHeartbeatFlag)    //�ɹ����յ�����
            {
                printmsg(ECU_DBG_OTHER,"Trina_Heartbeat Successful!!!\n");
                return 0;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        return 1;
    }else
    {
        //����ʧ�ܣ���Ҫ�����߳�
        restartThread(TYPE_TRINASOLAR);
        return -1;
    }
}
#ifdef THREAD_PRIORITY_TRINASOLAR
void TrinaSolar_thread_entry(void* parameter)	//����߳�
{
    rt_err_t result;
    int FailedDelayTime = 1;		//ʧ�ܵȴ�ʱ��	��һ��ʧ�ܵȴ�1���ӣ��ڶ���ʧ�ܵȴ�2���ӣ��Դ�����
    rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_TRINASOLAR);

    //�ж��Ƿ���иù���  �������δ�򿪣��˳����߳�  ������ܴ򿪣����и��߳�
    if(-1 == TrinaFunction())
    {
        printmsg(ECU_DBG_OTHER,"Trina Solar Function Close!\n");
        return;
    }

    initTrinaSolarSocketArgs();
    init_TrinaSendLock();
    printf("TrinaSolar_thread_entry init success\n");
    while(1)
    {
        if(TrinaSolarSocketFD >= 0) 	closesocket(TrinaSolarSocketFD);
        //�ȵ�¼����������¼�ɹ��󣬴������ܺ������߳�
        if(0 == Trina_Login())
        {
            //���������߳�
            rt_thread_detach(&recv_thread);
            /* init TrinaSolar thread */
            result = rt_thread_init(&recv_thread,"Trirecv",TrinaSolar_recv_thread_entry,RT_NULL,(rt_uint8_t*)&recv_stack[0],sizeof(recv_stack),THREAD_PRIORITY_TRINASOLAR_RECV,5);
            if (result == RT_EOK)	rt_thread_startup(&recv_thread);
            //���������߳�
            rt_thread_detach(&heartbeat_thread);
            /* init TrinaSolar thread */
            result = rt_thread_init(&heartbeat_thread,"Triheart",TrinaSolar_heartbeat_thread_entry,RT_NULL,(rt_uint8_t*)&heartbeat_stack[0],sizeof(heartbeat_stack),THREAD_PRIORITY_TRINASOLAR_HEARTBATE,5);
            if (result == RT_EOK)	rt_thread_startup(&heartbeat_thread);
            //������������
            while(1)
            {
                //����������������
                rt_thread_delay(FailedDelayTime * RT_TICK_PER_SECOND*60);
            }

        }else
        {
            closesocket(TrinaSolarSocketFD);
            TrinaSolarSocketFD = -1;
        }

        rt_thread_delay(FailedDelayTime * RT_TICK_PER_SECOND*60);
        FailedDelayTime *= 2;
        if(FailedDelayTime>=16)	FailedDelayTime = 16;
    }

}
#endif
void TrinaSolar_recv_thread_entry(void* parameter)	//��Ͻ����߳�
{
    int ret = 0;	

    while(1)
    {
        //�����¼�
        ret = TrinaRecvEvent();
        if(ret == 1)
        {
            //�ж���ʲô�¼�����ʲô����
            process_TrinaEvent(TrinaRecvSize,TrinaRecvBuff);
        }
    }
}

void TrinaSolar_heartbeat_thread_entry(void* parameter)	//��������߳�
{
    while(1)
    {
        Trina_Heartbeat();
        rt_thread_delay(RT_TICK_PER_SECOND*60);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void T_connect()
{
    Trina_socket_connect(TrinaSolar_arg.domain,TrinaSolar_arg.ip,TrinaSolar_arg.port1);
}
void T_send(char *buff,int size)
{
    Trina_SendData(TrinaSolarSocketFD,buff,size);
}
void T_recv(void)
{
    char recvbuffer[20];
    int size = 0;
    Trina_RecvData(TrinaSolarSocketFD, recvbuffer, &size, 10);
}

void T_login(void)
{
    Trina_Login();
}
void T_heart(void)
{
    Trina_Heartbeat();
}
void T_collect(void)
{
    Collect_TrinaSolar_Record();
}
FINSH_FUNCTION_EXPORT(T_connect, eg:T_connect());
FINSH_FUNCTION_EXPORT(T_send, eg:T_send());
FINSH_FUNCTION_EXPORT(T_recv, eg:T_recv());
FINSH_FUNCTION_EXPORT(T_login, eg:T_login());
FINSH_FUNCTION_EXPORT(T_heart, eg:T_heart());
FINSH_FUNCTION_EXPORT(T_collect, eg:T_collect());
#endif
