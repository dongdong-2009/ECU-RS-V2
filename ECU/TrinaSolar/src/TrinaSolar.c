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
#include "Serverfile.h"

#define TRINA_MAJOR_VERSION 0x01
#define TRINA_MINOR_VERSION 0x00
#define COMPANY_CODE    "TT"

extern ecu_info ecu;	//ecu相关信息
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
unsigned char TrinaSolarDataIndex = 1;
rt_mutex_t TrinaSend_lock = RT_NULL;        //sockect发送锁
Socket_Cfg TrinaSolar_arg;  //服务器配置信息
extern rt_mutex_t record_data_lock;

int TrinaSolarSocketFD = -1;		//天合SOCKET文件描述符
unsigned char TrinaSolarConnectFlag = 0;	//与服务器连接断开标志
unsigned char TrinaSolarHeartbeatFlag = 0;  //1表示接收到心跳了
unsigned char TrinaSolarDataFlag = 0;	//接受数据标志
unsigned char TrinaSolarFunctionFlag = 0;	//功能开关标志
unsigned char TrinaRecvBuff[4096];
int TrinaRecvSize = 0;

#ifdef THREAD_PRIORITY_TRINASOLAR_RECV
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t recv_stack[1024];
static struct rt_thread recv_thread;
#endif

#ifdef THREAD_PRIORITY_TRINASOLAR_HEARTBATE
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t heartbeat_stack[1024];
static struct rt_thread heartbeat_thread;
#endif


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

        //数据域
        //总输入电量		保留2位小数
        Trina_Data[58] = ((unsigned int)(ecu.life_energy*100)/16777216)%256;
        Trina_Data[59] = ((unsigned int)(ecu.life_energy*100)/65536)%256;
        Trina_Data[60] = ((unsigned int)(ecu.life_energy*100)/256)%256;
        Trina_Data[61] =  (unsigned int)(ecu.life_energy*100)%256;
        //总组件数
        Trina_Data[62] = ecu.validNum;

        //组件数
        Trina_Data[63] = ecu.count;
        //上报时间
        Trina_Data[64] = (ecu.curTime[0] - '0')*16+(ecu.curTime[1] - '0');
        Trina_Data[65] = (ecu.curTime[2] - '0')*16+(ecu.curTime[3] - '0');
        Trina_Data[66] = (ecu.curTime[4] - '0')*16+(ecu.curTime[5] - '0');
        Trina_Data[67] = (ecu.curTime[6] - '0')*16+(ecu.curTime[7] - '0');
        Trina_Data[68] = (ecu.curTime[8] - '0')*16+(ecu.curTime[9] - '0');
        Trina_Data[69] = (ecu.curTime[10] - '0')*16+(ecu.curTime[11] - '0');
        Trina_Data[70] = (ecu.curTime[12] - '0')*16+(ecu.curTime[13] - '0');
        //索引
        Trina_Data[71] = TrinaSolarDataIndex++;
        packlength = 72;
        for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)
        {
            //判断是否通讯上，且机型为jbox

            if((1 == curinverter->status.comm_failed3_status)&& (1 == curinverter->model))
            {
                //组件编号
                memcpy(&Trina_Data[packlength],curinverter->uid,12);
                packlength += 15;
                //设备类型
                Trina_Data[packlength++] = 3;
                //输入电压	保留1位小数
                Trina_Data[packlength++] = curinverter->PV1/256;
                Trina_Data[packlength++] = curinverter->PV1%256;
                //输入功率	保留1位小数
                Trina_Data[packlength++] = (int)curinverter->AveragePower1/256;
                Trina_Data[packlength++] = (int)curinverter->AveragePower1%256;
                //输出电流	保留1位小数
                Trina_Data[packlength++] = curinverter->PI_Output/256;
                Trina_Data[packlength++] = curinverter->PI_Output%256;
                //温度
                Trina_Data[packlength++] = curinverter->temperature - 100;
                //输入电量	保留6位小数
                energy = curinverter->EnergyPV1 / (3600000/1000000);
                Trina_Data[packlength++] = (energy/65536)%256;
                Trina_Data[packlength++] = (energy/256)%256;
                Trina_Data[packlength++] = energy%256;
                //RSD使能状态
                Trina_Data[packlength++] = curinverter->status.function_status;
            }
            curinverter++;

        }
        
        Trina_Data[56] = (packlength - 58)/256;
        Trina_Data[57] = (packlength - 58)%256;
        CRC = computeCRC((unsigned char*)Trina_Data, packlength);

        Trina_Data[packlength++] = (CRC >> 8);
        Trina_Data[packlength++] = (CRC & 0xFF);


        if(ecu.count > 0)
        {
            printf("%d\n",packlength);
            for( i =0;i<packlength;i++)
            {
                printf("%02x ",Trina_Data[i]);
            }
            printf("\n");
            save_TrinaSolar_Record(ecu.curTime,Trina_Data,packlength);
        }

        free(Trina_Data);
        Trina_Data = NULL;
    }
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
            TrinaSolarFunctionFlag = 1;
            return 1;
        }
    }
    TrinaSolarFunctionFlag = 0;
    return -1;
}

int GetTrinaSolarNewRecord(char *newFile,int *flag)
{
    DIR *dirp;
    struct dirent *d;
    int num = 0;
    char path[100] , fullpath[100] = {'\0'};
    int fileDate = 0,temp = 0;
    int fileTime = 0,tempT = 0;
    char tempDate[9] = {'\0'};
    char tempTime[4] = {'\0'};
    rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
    if(result == RT_EOK)
    {
        /* 打开dir目录*/
        dirp = opendir("/home/record/tridata");

        if(dirp == RT_NULL)
        {
            printmsg(ECU_DBG_OTHER,"check Old File open directory error");
            mkdir("/home/record/tridata",0);
        }
        else
        {
            /* 读取dir目录*/
            while ((d = readdir(dirp)) != RT_NULL)
            {
                num ++;
                memcpy(tempDate,d->d_name,8);
                tempDate[8] = '\0';
                memcpy(tempTime,&d->d_name[9],3);
                tempTime[3] = '\0';
                temp = atoi(tempDate);
                tempT = atoi(tempTime);

                if((temp >= fileDate) || (fileDate == 0))
                {
                    if(tempT > fileTime)
                    {
                        fileDate = temp;
                        fileTime = 	tempT;
                        memset(path,0,100);
                        strcpy(path,d->d_name);
                    }else
                    {
                        if(fileDate == 0)
                        {
                            fileDate = temp;
                            fileTime = 	tempT;
                            memset(path,0,100);
                            strcpy(path,d->d_name);
                        }

                    }
                    
                }

            }
            if(fileDate != 0)
            {
                if(num > 1)
                {
                    *flag = 1;
                }else
                {
                    *flag = 0;
                }

                sprintf(fullpath,"%s/%s","/home/record/tridata",path);
                strcpy(newFile,fullpath);
                closedir(dirp);
                rt_mutex_release(record_data_lock);
                return 1;
            }
            /* 关闭目录 */
            closedir(dirp);
        }
    }
    rt_mutex_release(record_data_lock);
    return 0;
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
        return -1;
    }
    //互斥量
    result = rt_mutex_take(TrinaSend_lock, RT_WAITING_FOREVER);
    if(result == RT_EOK)
    {
        send_count = send(sockfd, sendbuffer, size, 0);
        //printf("send_count:%d\n",send_count);
        if(send_count < 0)  //发送数据小于0,表示当前状态与服务器未连接
        {
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
        return -6;
    case 0:
        //printf("Trina_RecvData Receive date timeout\n");
        return -5;
    default:
        if(FD_ISSET(sockfd, &rd)){
            memset(recvbuffer, '\0', sizeof(recvbuffer));
            recv_count = recv(sockfd, recvbuffer, 4096, 0);
            recvbuffer[recv_count] = '\0';
            *size = recv_count;
            //printf("Trina_RecvData:%d  %s\n", *size,recvbuffer);

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

int TrinaRecvEvent(void)
{
    unsigned short CRC_calc = 0,CRC_read = 0,CRClen = 0;
    if(Trina_RecvData(TrinaSolarSocketFD, (char*)TrinaRecvBuff, &TrinaRecvSize, 2) > 0)
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
            return -2; //接收数据校验失败
        }
    }else
    {
        return -1;  //接收数据失败
    }
}

int Trina_SendRecord(char *sendbuff,int length)
{
    int index = 0;

    TrinaSolarDataFlag = 0;
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, length) >= 0)
    {
        for(index = 0;index<3;index++)
        {
            if(1 == TrinaSolarDataFlag)    //成功接收到
            {
                printmsg(ECU_DBG_OTHER,"Trina_SendData Successful!!!\n");
                return 0;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        return 1;
    }else
    {
        return -1;
    }

}

int TrinaSolar_Response(unsigned short CMD,int length,char *buff)
{
    char sendbuff[256] = {'\0'};
    unsigned short CRC = 0;

    sendbuff[0] = 'T';
    sendbuff[1] = 'S';
    memcpy(&sendbuff[2],ecu.ECUID12,12);
    memcpy(&sendbuff[27],ecu.ECUID12,12);
    sendbuff[52] = CMD/256;
    sendbuff[53] = CMD%256;
    sendbuff[54] = TRINA_MAJOR_VERSION;
    sendbuff[55] = TRINA_MINOR_VERSION;
    sendbuff[56] = length/256;
    sendbuff[57] = length%256;
    memcpy(&sendbuff[58],buff,length);

    CRC = computeCRC((unsigned char*)sendbuff, (58+length));

    sendbuff[58+length] = (CRC >> 8);
    sendbuff[59+length] = (CRC & 0xFF);
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, 60) < 0)
    {
        //发送失败，需要重启线程
        TrinaSolarConnectFlag = 0;
        restartThread(TYPE_TRINASOLAR);
        return -1;
    }

    return 1;
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
    }else if(TrinaCMD == 0x0A01)
    {
        TrinaSolarDataFlag = 1;
    }else if(TrinaCMD == 0x0004)	//设置服务器
    {
        char IP[50] = {'\0'};
        int port = 0;
        char str[200] = {'\0'};
        memcpy(IP,&recvbuffer[58],50);
        port = recvbuffer[108] * 0x100+recvbuffer[109];
        sprintf(str,"Domain=%s\nIP=%s\nPort1=%d\nPort2=%d\n",IP,TrinaSolar_arg.ip,port,port);
        echo("/config/trina.con",str);

    }else if(TrinaCMD == 0x0005)	//下发升级通知
    {

    }else if(TrinaCMD == 0x0009)	//重启ECU
    {

    }else if(TrinaCMD == 0x000A)	//获取设备信息
    {

    }else if(TrinaCMD == 0x0202)	//设置RSD功能
    {

    }else if(TrinaCMD == 0x0203)	//重启组件
    {

    }else if(TrinaCMD == 0x0204)	//固件升级
    {

    }else if(TrinaCMD == 0x0205)	//获取ECU采集器组件对应的关系
    {

    }

}

int checkTrinaSolarDataFormat(char *data,int length)
{
    unsigned short CRC_calc = 0,CRC_read = 0,CRClen = 0;
    if((data[52] == 0x02) &&(data[53] == 0x01))
    {
        CRClen = length-2;
        CRC_calc = computeCRC((unsigned char *)data, CRClen);
        CRC_read = ((((unsigned short)data[CRClen] << 8) & 0xff00)
                    | ((unsigned short)data[CRClen +1] & 0x00ff));

        if(CRC_calc == CRC_read)
        {
            return 1;
        }else
        {
            return -2; //接收数据校验失败
        }
    }else
    {
        return -1;
    }
}

void process_TrinaData(void)
{
    char recordpath[100] = { '\0' };
    int fd = 0,flag = 0;
    int ret = 0;
    int length = 0;
    char *trina_data = NULL;
    trina_data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);

    while(1)
    {
        ret = GetTrinaSolarNewRecord(recordpath,&flag);
        if(ret == 1)	//存在数据
        {
            fd = open(recordpath, O_RDONLY, 0);
            if(fd >= 0)
            {
                memset(trina_data, '\0', CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
                length = read(fd, trina_data, CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
                close(fd);
                //判断读到的格式是否正确，不正确直接删除文件
                if(1 == checkTrinaSolarDataFormat(trina_data,length))
                {
                    if(0 == Trina_SendRecord(trina_data,length))
                    {
                        unlink(recordpath);
                    }else
                    {
                        break;
                    }
                    if(flag == 0)
                    {
                        break;
                    }
                }
                else
                {
                    unlink(recordpath);
                }
                rt_thread_delay(10);

            }
        }else
        {

            break;
        }
    }
    free(trina_data);
    trina_data = NULL;


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
            if(1 == TrinaSolarHeartbeatFlag)    //成功接收到心跳
            {
                printmsg(ECU_DBG_OTHER,"Trina_Heartbeat Successful!!!\n");
                return 0;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        return 1;
    }else
    {
        //发送失败，需要重启线程
        TrinaSolarConnectFlag = 0;
        restartThread(TYPE_TRINASOLAR);
        return -1;
    }
}
#ifdef THREAD_PRIORITY_TRINASOLAR
void TrinaSolar_thread_entry(void* parameter)	//天合线程
{
    rt_err_t result;
    //   int FailedDelayTime = 1;		//失败等待时间	第一次失败等待1分钟，第二次失败等待2分钟，以此类推
    TrinaSolarConnectFlag = 0;

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

        if(TrinaSolarSocketFD >= 0)
        {
            closesocket(TrinaSolarSocketFD);
            TrinaSolarSocketFD = -1;
        }
        //先登录服务器，登录成功后，创建接受和心跳线程
        if(0 == Trina_Login())
        {
            //创建接受线程
            /* init TrinaSolar thread */
            rt_thread_detach(&recv_thread);

            result = rt_thread_init(&recv_thread,"Trirecv",TrinaSolar_recv_thread_entry,RT_NULL,(rt_uint8_t*)&recv_stack[0],sizeof(recv_stack),THREAD_PRIORITY_TRINASOLAR_RECV,5);
            if (result == RT_EOK)	rt_thread_startup(&recv_thread);
            
            //创建心跳线程
            /* init TrinaSolar thread */
            rt_thread_detach(&heartbeat_thread);
            result = rt_thread_init(&heartbeat_thread,"Triheart",TrinaSolar_heartbeat_thread_entry,RT_NULL,(rt_uint8_t*)&heartbeat_stack[0],sizeof(heartbeat_stack),THREAD_PRIORITY_TRINASOLAR_HEARTBATE,5);
            if (result == RT_EOK)	rt_thread_startup(&heartbeat_thread);
            //正常发送数据
            while(1)
            {
                process_TrinaData();
                //发送正常发电数据
                rt_thread_delay(RT_TICK_PER_SECOND*60);
            }

        }else
        {
            printf("Trina_Login failed\n");
            closesocket(TrinaSolarSocketFD);
            TrinaSolarSocketFD = -1;
        }

        rt_thread_delay(RT_TICK_PER_SECOND*60);
    }

}
#endif
void TrinaSolar_recv_thread_entry(void* parameter)	//天合接受线程
{
    int ret = 0;

    while(1)
    {
        //接受事件
        ret = TrinaRecvEvent();
        if(ret == 1)
        {
            //判断是什么事件，做什么事情
            process_TrinaEvent(TrinaRecvSize,TrinaRecvBuff);
        }
        if(TrinaSolarConnectFlag == 0)
        {
            printf("TrinaSolar_recv_thread_entry over\n");
            if(TrinaSolarSocketFD>=0)
            {
                closesocket(TrinaSolarSocketFD);
                TrinaSolarSocketFD = -1;
            }

            return;
        }
    }
}

void TrinaSolar_heartbeat_thread_entry(void* parameter)	//天合心跳线程
{
    int index =0;
    rt_thread_delay(RT_TICK_PER_SECOND*20);
    while(1)
    {
        Trina_Heartbeat();
        for(index = 0;index <60;index++)
        {
            if(TrinaSolarConnectFlag == 0)
            {
                printf("TrinaSolar_heartbeat_thread_entry over\n");
                if(TrinaSolarSocketFD>=0)
                {
                    closesocket(TrinaSolarSocketFD);
                    TrinaSolarSocketFD = -1;
                }

                return;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        
    }
}

#if 0
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
void getTri(void)
{
    char path[100] = { '\0' };
    int flag = 0;
    int ret = GetTrinaSolarNewRecord(path,&flag);
    printf("ret:%d %s %d\n",ret,path,flag);
}

void proTri(void)
{
    process_TrinaData();
}
FINSH_FUNCTION_EXPORT(proTri, eg:proTri);
FINSH_FUNCTION_EXPORT(getTri, eg:getTri);
FINSH_FUNCTION_EXPORT(T_connect, eg:T_connect());
FINSH_FUNCTION_EXPORT(T_send, eg:T_send());
FINSH_FUNCTION_EXPORT(T_recv, eg:T_recv());
FINSH_FUNCTION_EXPORT(T_login, eg:T_login());
FINSH_FUNCTION_EXPORT(T_heart, eg:T_heart());
FINSH_FUNCTION_EXPORT(T_collect, eg:T_collect());
#endif
#endif
