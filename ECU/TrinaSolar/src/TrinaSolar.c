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
#include "remoteUpdate.h"
#include "rsdFunction.h"

#define TRINA_MAJOR_VERSION 0x01
#define TRINA_MINOR_VERSION 0x00
#define COMPANY_CODE    "TA"

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
unsigned char TrinaSolarTimingFlag = 0;	//功能开关标志
char TrinaSolarSetTime[15] = {'\0'};
unsigned char TrinaRecvBuff[4096];
int TrinaRecvSize = 0;

#ifdef THREAD_PRIORITY_TRINASOLAR_RECV
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t recv_stack[2048];
static struct rt_thread recv_thread;
#endif

#ifdef THREAD_PRIORITY_TRINASOLAR_HEARTBATE
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t heartbeat_stack[1024];
static struct rt_thread heartbeat_thread;
#endif

void insertTrinaSetRSDInfo(unsigned short num,int RSDFunctionStatus,char *IDStr)
{
    int i;
    char inverter_id[16] = {'\0'};
    int fd;
    char buff[50];
    fd = open("/tmp/setrsd", O_WRONLY | O_APPEND | O_CREAT,0);
    if (fd >= 0)
    {
        for(i=0; i<num; i++)
        {
            memcpy(inverter_id,&IDStr[i*15],15);

            if(RSDFunctionStatus == 0)//关闭RSD功能
            {
                sprintf(buff,"%s,2,0,000\n",inverter_id);
            }else//开启RSD功能
            {
                sprintf(buff,"%s,1,1,000\n",inverter_id);
            }

            write(fd,buff,strlen(buff));
        }

        close(fd);
    }

}

void insertTrinaRSDCon(unsigned short num,int RSDFunctionStatus,char *IDStr)
{
    int i;
    char inverter_id[16] = {'\0'};
    int fd;
    char buff[50];
    fd = open("/tmp/rsdcon.con", O_WRONLY  | O_APPEND | O_CREAT,0);
    if (fd >= 0)
    {
        for(i=0; i<num; i++)
        {
            memcpy(inverter_id,&IDStr[i*15],15);

            if(RSDFunctionStatus == 0)//关闭RSD功能
            {
                sprintf(buff,"%s,0\n",inverter_id);
            }else//开启RSD功能
            {
                sprintf(buff,"%s,1\n",inverter_id);
            }

            write(fd,buff,strlen(buff));
        }

        close(fd);
    }

}


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

//初始化通讯参数
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
            if(!strncmp(buff, "Port", 4))
            {
                TrinaSolar_arg.port1=atoi(&buff[5]);
                TrinaSolar_arg.port2=atoi(&buff[5]);
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

//存储天合数据格式
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

//采集天合数据
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
        Trina_Data[packlength++] = (CRC & 0xFF);
        Trina_Data[packlength++] = (CRC >> 8);
        


        if(ecu.count > 0)
        {
            save_TrinaSolar_Record(ecu.curTime,Trina_Data,packlength);
        }

        free(Trina_Data);
        Trina_Data = NULL;
    }
}

//查看天合服务器功能
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

//获取天合最新的数据
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
        printhexmsg(ECU_DBG_COMM,"Send", (char *)sendbuffer, size);
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
            printhexmsg(ECU_DBG_COMM,"Recv", (char *)recvbuffer, *size);
            //printf("Trina_RecvData:%d  %s\n", *size,recvbuffer);
            if(recv_count <= 0)	//接收到的字符小于等于0
            {
                TrinaSolarConnectFlag = 0;
            }
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
        sendbuff[111] = (CRC & 0xFF);
        sendbuff[112] = (CRC >> 8);
        
        if(Trina_SendData(TrinaSolarSocketFD, sendbuff, 113) >= 0)
        {
            if(Trina_RecvData(TrinaSolarSocketFD, (char*)TrinaRecvBuff, &TrinaRecvSize, 10) > 0)
            {
                CRClen = TrinaRecvSize-2;
                CRC_calc = computeCRC(TrinaRecvBuff, CRClen);
                CRC_read = ((((unsigned short)TrinaRecvBuff[CRClen+1] << 8) & 0xff00)
                            | ((unsigned short)TrinaRecvBuff[CRClen ] & 0x00ff));
                printf("CRC_calc:%x CRC_read:%x %x %x\n",CRC_calc,CRC_read,TrinaRecvBuff[CRClen],TrinaRecvBuff[CRClen +1]);
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

//天合接受时间
int TrinaRecvEvent(void)
{
    unsigned short CRC_calc = 0,CRC_read = 0,CRClen = 0;
    if(Trina_RecvData(TrinaSolarSocketFD, (char*)TrinaRecvBuff, &TrinaRecvSize, 4) > 0)
    {
        CRClen = TrinaRecvSize-2;
        CRC_calc = computeCRC(TrinaRecvBuff, CRClen);
        CRC_read = ((((unsigned short)TrinaRecvBuff[CRClen+1] << 8) & 0xff00)
                    | ((unsigned short)TrinaRecvBuff[CRClen ] & 0x00ff));
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
    sendbuff[58+length] = (CRC & 0xFF);
    sendbuff[59+length] = (CRC >> 8);
    
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, (60+length)) < 0)
    {
        //发送失败，需要重启线程
        TrinaSolarConnectFlag = 0;
        return -1;
    }

    return 1;
}

int TrinaSolar_Response_relation_ECU_Module(void)
{
    char *sendbuff = NULL;
    unsigned short CRC = 0;
    char curTime[15];
    inverter_info *curinverter = inverterInfo;
    int i = 0,packlen = 0;
    sendbuff = malloc(4096);
    memset(sendbuff,0x00,4096);

    sendbuff[0] = 'T';
    sendbuff[1] = 'S';
    memcpy(&sendbuff[2],ecu.ECUID12,12);
    memcpy(&sendbuff[27],ecu.ECUID12,12);
    sendbuff[52] = 0x0a;
    sendbuff[53] = 0x05;
    sendbuff[54] = TRINA_MAJOR_VERSION;
    sendbuff[55] = TRINA_MINOR_VERSION;
    sendbuff[56] = 0;
    sendbuff[57] = 0;

    apstime(curTime);
    curTime[14] = '\0';
    sendbuff[58] = ecu.validNum;
    sendbuff[59] = ecu.count;

    sendbuff[60] = (curTime[0] - '0')*16+(curTime[1] - '0');
    sendbuff[61] = (curTime[2] - '0')*16+(curTime[3] - '0');
    sendbuff[62] = (curTime[4] - '0')*16+(curTime[5] - '0');
    sendbuff[63] = (curTime[6] - '0')*16+(curTime[7] - '0');
    sendbuff[64] = (curTime[8] - '0')*16+(curTime[9] - '0');
    sendbuff[65] = (curTime[10] - '0')*16+(curTime[11] - '0');
    sendbuff[66] = (curTime[12] - '0')*16+(curTime[13] - '0');
    sendbuff[67] = 1;
    packlen = 68;
    for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)
    {
        //判断是否通讯上，且机型为jbox
        if((1 == curinverter->status.comm_failed3_status))
        {
            memcpy(&sendbuff[packlen],curinverter->uid,12);
            packlen += 15;
            sendbuff[packlen++] = curinverter->version/256;
            sendbuff[packlen++] = curinverter->version%256;
            printf("version:%d\n",curinverter->version);
        }

    }
    sendbuff[56] = (packlen-58)/256;
    sendbuff[57] = (packlen-58)%256;


    CRC = computeCRC((unsigned char*)sendbuff, packlen);
    sendbuff[packlen] = (CRC & 0xFF);
    sendbuff[packlen+1] = (CRC >> 8);
    
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, (packlen+2)) < 0)
    {
        //发送失败，需要重启线程
        TrinaSolarConnectFlag = 0;
        free(sendbuff);
        sendbuff = NULL;
        return -1;
    }
    free(sendbuff);
    sendbuff = NULL;
    return 1;

}

int ResolveURI(char *URI,char *Domain,unsigned short *port,char *RemotePath)
{
    //ftp://106.14.174.201:21/6-10Ktianhe_Ver.0F.bin
    int index = 0,colonIndex;
    char PortStr[5] = {'\0'};
    for(index = 6;index < strlen(URI);index++)
    {
        if(URI[index] == ':')
        {
            colonIndex = index;
            memcpy(Domain,&URI[6],(index-6));
        }

        if(URI[index] == '/')
        {
            memcpy(PortStr,&URI[colonIndex+1],(index - colonIndex -1));
            *port = atoi(PortStr);
            memcpy(RemotePath,&URI[index],(strlen(URI) - index));
            return 0;
        }

    }

    return -1;
    
}

//处理天合事件
void process_TrinaEvent(int Data_Len,const unsigned char *recvbuffer)
{
    unsigned short TrinaCMD = 0;
    char str[200] = {'\0'};
    TrinaCMD = recvbuffer[52] * 256 + recvbuffer[53];
    printf("TrinaCMD:%x\n",TrinaCMD);

    if(TrinaCMD == 0x0801)
    {

    }else if(TrinaCMD == 0x0802)
    {
        TrinaSolarHeartbeatFlag = 1;

    }else if(TrinaCMD == 0x0803)
    {
        TrinaSolarTimingFlag = 1;
        //获取时间
        TrinaSolarSetTime[0] = recvbuffer[58]/16 + '0';
        TrinaSolarSetTime[1] = recvbuffer[58]%16 + '0';
        TrinaSolarSetTime[2] = recvbuffer[59]/16 + '0';
        TrinaSolarSetTime[3] = recvbuffer[59]%16 + '0';
        TrinaSolarSetTime[4] = recvbuffer[60]/16 + '0';
        TrinaSolarSetTime[5] = recvbuffer[60]%16 + '0';
        TrinaSolarSetTime[6] = recvbuffer[61]/16 + '0';
        TrinaSolarSetTime[7] = recvbuffer[61]%16 + '0';
        TrinaSolarSetTime[8] = recvbuffer[62]/16 + '0';
        TrinaSolarSetTime[9] = recvbuffer[62]%16 + '0';
        TrinaSolarSetTime[10] = recvbuffer[63]/16 + '0';
        TrinaSolarSetTime[11] = recvbuffer[63]%16 + '0';
        TrinaSolarSetTime[12] = recvbuffer[64]/16 + '0';
        TrinaSolarSetTime[13] = recvbuffer[64]%16 + '0';
        TrinaSolarSetTime[14] = '\0';


    }else if(TrinaCMD == 0x0A01)
    {
        TrinaSolarDataFlag = 1;

    }else if(TrinaCMD == 0x0004)	//设置服务器
    {
        char IP[50] = {'\0'};
        int port = 0;

        memcpy(IP,&recvbuffer[58],50);
        port = recvbuffer[108] * 0x100+recvbuffer[109];
        sprintf(str,"Domain=%s\nIP=%s\nPort=%d\n",IP,TrinaSolar_arg.ip,port);
        echo("/config/trina.con",str);

        TrinaSolar_Response(0x0804,0,str);
        TrinaSolarConnectFlag = 0;

    }else if(TrinaCMD == 0x0005)	//下发升级通知
    {	//只识别升级ECU的
        char URI[100] = {'\0'};		//ftp://106.14.174.201:21/6-10Ktianhe_Ver.0F.bin
        char user[10] = {'\0'};
        char password[10] = {'\0'};
        char Domain[50] = {'\0'};
        unsigned short port = 0;
        char remotepath[50] = {'\0'};
        if(recvbuffer[58] == 0x01)	//FTP升级
        {
            if(recvbuffer[59] == 0x02)	//ECU升级
            {
                memcpy(URI,&recvbuffer[60],100);
                memcpy(user,&recvbuffer[160],6);
                memcpy(password,&recvbuffer[166],6);
                TrinaSolar_Response(0x0805,0,str);
                //解析URI
                ResolveURI(URI,Domain,&port,remotepath);
                printf("Domain:%s port:%d path:%s user:%s passwd:%s\n",Domain,port,remotepath,user,password);
                optimizeFileSystem(300);
                updateECUByTrinaSolar(Domain,Domain, port, user, password,remotepath);
            }
        }

    }else if(TrinaCMD == 0x0008)	//重启ECU
    {
        TrinaSolar_Response(0x0808,0,str);
        reboot();

    }else if(TrinaCMD == 0x0009)	//获取设备信息
    {
        char curTime[15] = {'\0'};
        apstime(curTime);
        curTime[14] = '\0';

        memset(str,0x00,200);
        //厂家        2字节 58
        str[0] = COMPANY_CODE[0];
        str[1] = COMPANY_CODE[1];
        //型号       13字节    60
        memcpy(&str[2],ECU_VERSION,strlen(ECU_VERSION));

        //CCID       20字节   73

        //设备时间    7字节    93
        str[35] = (curTime[0] - '0')*16+(curTime[1] - '0');
        str[36] = (curTime[2] - '0')*16+(curTime[3] - '0');
        str[37] = (curTime[4] - '0')*16+(curTime[5] - '0');
        str[38] = (curTime[6] - '0')*16+(curTime[7] - '0');
        str[39] = (curTime[8] - '0')*16+(curTime[9] - '0');
        str[40] = (curTime[10] - '0')*16+(curTime[11] - '0');
        str[41] = (curTime[12] - '0')*16+(curTime[13] - '0');

        //固件版本    10字节    100
        sprintf(&str[42],"%s.%s",MAJORVERSION,MINORVERSION);
        //国家标准    1字节   110
        str[52] =0x01;
        TrinaSolar_Response(0x0809,53,str);

    }else if(TrinaCMD == 0x0202)	//设置RSD功能
    {
        if(recvbuffer[58] == 0x00)	//全部设置
        {
            if(recvbuffer[59] == 0x01)	//使能
            {
                save_rsdFunction_change_flag();
                saveChangeFunctionStatus(1,2,0);//打开心跳功能
                unlink("/tmp/setrsd");
                unlink("/tmp/rsdcon.con");
                //重启采集线程
                restartThread(TYPE_DATACOLLECT);
            }else	//禁能
            {
                save_rsdFunction_change_flag();
                saveChangeFunctionStatus(2,0,0);//关闭心跳功能
                unlink("/tmp/setrsd");
                unlink("/tmp/rsdcon.con");
                //重启采集线程
                restartThread(TYPE_DATACOLLECT);
            }
        }else		//部分设置
        {
            unsigned short num = recvbuffer[58];
            int RSDFunctionStatus = recvbuffer[59];
            insertTrinaSetRSDInfo(num ,RSDFunctionStatus,(char *)&recvbuffer[60]);
            insertTrinaRSDCon(num ,RSDFunctionStatus,(char *)&recvbuffer[60]);
            //重启main线程
            restartThread(TYPE_DATACOLLECT);
        }

        str[0] = 0x01;
        TrinaSolar_Response(0x0a02,1,str);
    }else if(TrinaCMD == 0x0205)	//获取ECU采集器组件对应的关系
    {
        TrinaSolar_Response_relation_ECU_Module();
    }

}

int checkTrinaSolarDataFormat(char *data,int length)
{
    unsigned short CRC_calc = 0,CRC_read = 0,CRClen = 0;
    if((data[52] == 0x02) &&(data[53] == 0x01))
    {
        CRClen = length-2;
        CRC_calc = computeCRC((unsigned char *)data, CRClen);
        CRC_read = ((((unsigned short)data[CRClen +1] << 8) & 0xff00)
                    | ((unsigned short)data[CRClen] & 0x00ff));

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
    int index = 0,sendIndex = 0;
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
    sendbuff[58] = (CRC & 0xFF);
    sendbuff[59] = (CRC >> 8);
    
    TrinaSolarHeartbeatFlag = 0;
    for(sendIndex = 0;sendIndex < 3;sendIndex++)
    {
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
        }else
        {
            //发送失败，需要重启线程
            TrinaSolarConnectFlag = 0;
            return -1;
        }
    }
	
    TrinaSolarConnectFlag = 0;
    return -1;    
}

//校时
int Trina_Timing(void)
{
    int index = 0;
    char sendbuff[128] = { 0x00 };
    unsigned short CRC = 0;
    char curTime[15] = {'\0'};
    sendbuff[0] = 'T';
    sendbuff[1] = 'S';
    memcpy(&sendbuff[2],ecu.ECUID12,12);
    memcpy(&sendbuff[27],ecu.ECUID12,12);
    sendbuff[52] = 0x00;
    sendbuff[53] = 0x03;
    sendbuff[54] = TRINA_MAJOR_VERSION;
    sendbuff[55] = TRINA_MINOR_VERSION;
    sendbuff[56] = 0;
    sendbuff[57] = 0;

    CRC = computeCRC((unsigned char*)sendbuff, 58);
    sendbuff[58] = (CRC & 0xFF);
    sendbuff[59] = (CRC >> 8);
    
    TrinaSolarTimingFlag = 0;
    if(Trina_SendData(TrinaSolarSocketFD, sendbuff, 60) >= 0)
    {
        for(index = 0;index<3;index++)
        {
            if(1 == TrinaSolarTimingFlag)    //成功接收到心跳
            {
                printmsg(ECU_DBG_OTHER,"Trina_Timing Successful!!!\n");
                //与现在的时间进行对比
                apstime(curTime);
                if(memcmp(TrinaSolarSetTime,curTime,12))
                {
                    printf("TrinaSolarSetTime : %s\n",TrinaSolarSetTime);
                    set_time(TrinaSolarSetTime);
                    //重启采集线程
                    restartThread(TYPE_DATACOLLECT);
                }

                return 0;
            }
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
        return 1;
    }else
    {
        TrinaSolarTimingFlag = 0;
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
            //校时命令
            Trina_Timing();
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

        if(TrinaSolarConnectFlag == 0)	//判断是否断开连接
        {
            printf("TrinaSolar_recv_thread_entry over\n");
            if(TrinaSolarSocketFD>=0)
            {
                closesocket(TrinaSolarSocketFD);
                TrinaSolarSocketFD = -1;
                //关闭心跳线程，重启主线程
                rt_thread_detach(&heartbeat_thread);
                restartThread(TYPE_TRINASOLAR);
            }

            return;
        }
    }
}

void TrinaSolar_heartbeat_thread_entry(void* parameter)	//天合心跳线程
{
    rt_thread_delay(RT_TICK_PER_SECOND*20);
    while(1)
    {
        Trina_Heartbeat();
        rt_thread_delay(RT_TICK_PER_SECOND*120);

    }
}

#if 1
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
void proTiming()
{
    Trina_Timing();
}

void testURI(char *URI)
{

    char Domain[50] = {'\0'};
    unsigned short port = 0;
    char remotepath[50] = {'\0'};
    ResolveURI(URI,Domain,&port,remotepath);
    printf("Domain:%s port:%d remotepath:%s\n",Domain,port,remotepath);

}
FINSH_FUNCTION_EXPORT(proTri, eg:proTri);
FINSH_FUNCTION_EXPORT(getTri, eg:getTri);
FINSH_FUNCTION_EXPORT(T_connect, eg:T_connect());
FINSH_FUNCTION_EXPORT(T_send, eg:T_send());
FINSH_FUNCTION_EXPORT(T_recv, eg:T_recv());
FINSH_FUNCTION_EXPORT(T_login, eg:T_login());
FINSH_FUNCTION_EXPORT(T_heart, eg:T_heart());
FINSH_FUNCTION_EXPORT(T_collect, eg:T_collect());
FINSH_FUNCTION_EXPORT(testURI, eg:testURI());
FINSH_FUNCTION_EXPORT(proTiming, eg:proTiming());
#endif
#endif
