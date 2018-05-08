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

extern ecu_info ecu;	//ecu相关信息
Socket_Cfg TrinaSolar_arg;
int TrinaSolarSocketFD = -1;		//天合SOCKET文件描述符
unsigned char TrinaSolarConnectFlag = 0;	//与服务器连接断开标志
unsigned char TrinaSolarHeartbeatFlag = 0;  //1表示接收到心跳了
rt_mutex_t TrinaSend_lock = RT_NULL;        //sockect发送锁
unsigned char TrinaRecvBuff[4096];
int TrinaRecvSize = 0;

void init_TrinaSendLock(void)      //初始化TrinaSolar发送锁
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
    //初始化电量上传参数
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

int TrinaFunction(void)	//查看天合服务器功能是否打开
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
            return 1;
        }
    }
    return -1;
}

//创建一个socket连接服务器并返回
int Trina_socket_connect(char *domain,char *IP,int port)
{
    char IPaddr[20];
    struct sockaddr_in serv_addr;
    struct hostent * host;
    TrinaSolarSocketFD = socket(AF_INET,SOCK_STREAM,0);  //创建SOCKET
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
    //连接socket
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

//发送数据
int Trina_SendData(int sockfd, char *sendbuffer, int size)
{
    int send_count = 0;
    rt_err_t result;
    if(sockfd < 0)
    {
        TrinaSolarConnectFlag = 0;  //表示当前连接已经断开
        return -1;
    }
    //互斥量
    result = rt_mutex_take(TrinaSend_lock, RT_WAITING_FOREVER);
    if(result == RT_EOK)
    {
        send_count = send(sockfd, sendbuffer, size, 0);
        printf("send_count:%d\n",send_count);
        if(send_count < 0)  //发送数据小于0,表示当前状态与服务器未连接
        {
            TrinaSolarConnectFlag = 0;  //表示当前连接已经断开
            rt_mutex_release(TrinaSend_lock);
            return -1;
        }
    }
    rt_mutex_release(TrinaSend_lock);
    return send_count;
}
//接受数据
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
        printf("Trina_RecvData Receive date timeout\n");
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

int Trina_Login(void)   //登录
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
        //厂家        2字节 58
        sendbuff[58] = COMPANY_CODE[0];
        sendbuff[59] = COMPANY_CODE[1];
        //型号       13字节    60
        memcpy(&sendbuff[60],ECU_VERSION,strlen(ECU_VERSION));

        //CCID       20字节   73

        //设备时间    7字节    93
        sendbuff[93] = (curTime[0] - '0')*16+(curTime[1] - '0');
        sendbuff[94] = (curTime[2] - '0')*16+(curTime[3] - '0');
        sendbuff[95] = (curTime[4] - '0')*16+(curTime[5] - '0');
        sendbuff[96] = (curTime[6] - '0')*16+(curTime[7] - '0');
        sendbuff[97] = (curTime[8] - '0')*16+(curTime[9] - '0');
        sendbuff[98] = (curTime[10] - '0')*16+(curTime[11] - '0');
        sendbuff[99] = (curTime[12] - '0')*16+(curTime[13] - '0');

        //固件版本    10字节    100
        sprintf(&sendbuff[100],"%s.%s",MAJORVERSION,MINORVERSION);
        //国家标准    1字节   110
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
                    return -4; //接收数据校验失败
                }
            }else
            {
                return -3;  //接收数据失败
            }

        }else
        {
            return -2;  //发送数据失败
        }

    }else
    {
        return -1;      //连接服务器失败
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
    sendbuff[52] = 0x08;
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
            if(1 == TrinaSolarHeartbeatFlag)    //成功接收到心跳
            {
                return 0;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        return 1;
    }else
    {
        //发送失败，需要重启线程
        restartThread(TYPE_TRINASOLAR);
        return -1;
    }
}

void TrinaSolar_thread_entry(void* parameter)	//天合线程
{
    int FailedDelayTime = 1;		//失败等待时间	第一次失败等待1分钟，第二次失败等待2分钟，以此类推
    rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_TRINASOLAR);

    //判断是否带有该功能  如果功能未打开：退出该线程  如果功能打开：运行该线程
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
        //先登录服务器，登录成功后，创建接受和心跳线程
        if(0 == Trina_Login())
        {
            //创建接受线程

            //创建心跳线程

            //正常发送数据
            while(1)
            {

                rt_thread_delay(FailedDelayTime * RT_TICK_PER_SECOND*60);
            }

        }

        rt_thread_delay(FailedDelayTime * RT_TICK_PER_SECOND*60);
        FailedDelayTime *= 2;
        if(FailedDelayTime>=16)	FailedDelayTime = 16;
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
FINSH_FUNCTION_EXPORT(T_connect, eg:T_connect());
FINSH_FUNCTION_EXPORT(T_send, eg:T_send());
FINSH_FUNCTION_EXPORT(T_recv, eg:T_recv());
FINSH_FUNCTION_EXPORT(T_login, eg:T_login());
FINSH_FUNCTION_EXPORT(T_heart, eg:T_heart());
#endif
