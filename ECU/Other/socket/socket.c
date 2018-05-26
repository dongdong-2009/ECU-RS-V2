#include "socket.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "lan8720rst.h"
#include <stdio.h>
#include "rtc.h"
#include "threadlist.h"
#include "usart5.h"
#include "debug.h"
#include "string.h"
#include "rthw.h"
#include "rtthread.h"
#include "datetime.h"

extern unsigned char LED_Status;
Socket_Cfg client_arg;
Socket_Cfg control_client_arg;

void initSocketArgs(void)
{
    FILE *fp;
    char *buff = NULL;
    client_arg.port1 = CLIENT_SERVER_PORT1;
    client_arg.port2 = CLIENT_SERVER_PORT2;
    strcpy(client_arg.domain, CLIENT_SERVER_DOMAIN);
    strcpy(client_arg.ip, CLIENT_SERVER_IP);

    buff = malloc(512);
    memset(buff,'\0',512);
    //初始化电量上传参数
    fp = fopen("/config/datacent.con", "r");
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
                strcpy(client_arg.domain, &buff[7]);
                if('\n' == client_arg.domain[strlen(client_arg.domain)-1])
                    client_arg.domain[strlen(client_arg.domain)-1] = '\0';
            }
            if(!strncmp(buff, "IP", 2))
            {
                strcpy(client_arg.ip, &buff[3]);
                if('\n' == client_arg.ip[strlen(client_arg.ip)-1])
                    client_arg.ip[strlen(client_arg.ip)-1] = '\0';
            }
            if(!strncmp(buff, "Port1", 5))
                client_arg.port1=atoi(&buff[6]);
            if(!strncmp(buff, "Port2", 5))
                client_arg.port2=atoi(&buff[6]);
        }
        fclose(fp);
    }

    //初始化远程控制参数
    control_client_arg.port1 = CONTROL_SERVER_PORT1;
    control_client_arg.port2 = CONTROL_SERVER_PORT2;
    strcpy(control_client_arg.domain, CONTROL_SERVER_DOMAIN);
    strcpy(control_client_arg.ip, CONTROL_SERVER_IP);
    fp = fopen("/config/control.con", "r");
    if(fp)
    {
        while(1)
        {
            memset(buff, '\0',512);
            fgets(buff, 512, fp);
            if(!strlen(buff))
                break;
            if(!strncmp(buff, "Domain", 6))
            {
                strcpy(control_client_arg.domain, &buff[7]);
                if('\n' == control_client_arg.domain[strlen(control_client_arg.domain)-1])
                    control_client_arg.domain[strlen(control_client_arg.domain)-1] = '\0';
            }
            if(!strncmp(buff, "IP", 2))
            {
                strcpy(control_client_arg.ip, &buff[3]);
                if('\n' == control_client_arg.ip[strlen(control_client_arg.ip)-1])
                    control_client_arg.ip[strlen(control_client_arg.ip)-1] = '\0';
            }
            if(!strncmp(buff, "Port1", 5))
                control_client_arg.port1=atoi(&buff[6]);
            if(!strncmp(buff, "Port2", 5))
                control_client_arg.port2=atoi(&buff[6]);
        }
        fclose(fp);
    }

    printf("client_arg.domain:%s\n",client_arg.domain);
    printf("client_arg.ip:%s\n",client_arg.ip);
    printf("client_arg.port1:%d\n",client_arg.port1);
    printf("client_arg.port2:%d\n",client_arg.port2);

    printf("client_arg.domain:%s\n",control_client_arg.domain);
    printf("client_arg.ip:%s\n",control_client_arg.ip);
    printf("client_arg.port1:%d\n",control_client_arg.port1);
    printf("client_arg.port2:%d\n",control_client_arg.port2);


    free(buff);
    buff = NULL;
}


int randport(Socket_Cfg cfg)
{
    srand((unsigned)acquire_time());
    if(rand()%2)
        return cfg.port1;
    else
        return cfg.port2;
}


int createsocket(void)					//创建SOCKET连接
{
    int fd_sock;

    fd_sock=socket(AF_INET,SOCK_STREAM,0);
    if(-1==fd_sock)
        printmsg(ECU_DBG_OTHER,"Failed to create socket");
    else
        printmsg(ECU_DBG_OTHER,"Create socket successfully");

    return fd_sock;
}

int connect_client_socket(int fd_sock)				//通过有线的方式连接服务器
{	
    struct sockaddr_in serv_addr;
    struct hostent * host;

    //不存在有线连接，直接关闭socket
    if(rt_hw_GetWiredNetConnect() == 0)
    {
        closesocket(fd_sock);
        return -1;
    }


    host = gethostbyname(client_arg.domain);
    if(NULL == host)
    {
        printmsg(ECU_DBG_CLIENT,"Resolve domain failure");
    }
    else
    {
        memset(client_arg.ip, '\0', sizeof(client_arg.ip));
        sprintf(client_arg.ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
    }


    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(randport(client_arg));
    serv_addr.sin_addr.s_addr=inet_addr(client_arg.ip);
    memset(&(serv_addr.sin_zero),0,8);

    if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
        printmsg(ECU_DBG_CLIENT,"Failed to connect to EMA");
        closesocket(fd_sock);
        return -1;
    }
    else{
        printmsg(ECU_DBG_CLIENT,"Connect to EMA Client successfully");
        return 1;
    }
}

int connect_control_socket(int fd_sock)				//通过有线的方式连接服务器
{
    struct sockaddr_in serv_addr;
    struct hostent * host;

    //不存在有线连接，直接关闭socket
    if(rt_hw_GetWiredNetConnect() == 0)
    {
        closesocket(fd_sock);
        return -1;
    }

    host = gethostbyname(control_client_arg.domain);
    if(NULL == host)
    {
        printmsg(ECU_DBG_CONTROL_CLIENT,"Resolve domain failure");
    }
    else
    {
        memset(control_client_arg.ip, '\0', sizeof(control_client_arg.ip));
        sprintf(control_client_arg.ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
    }

    memset(&serv_addr,0,sizeof(struct sockaddr_in));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(randport(control_client_arg));
    serv_addr.sin_addr.s_addr=inet_addr(control_client_arg.ip);
    memset(&(serv_addr.sin_zero),0,8);

    if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
        printmsg(ECU_DBG_CONTROL_CLIENT,"Failed to connect to EMA");
        closesocket(fd_sock);
        return -1;
    }
    else{
        printmsg(ECU_DBG_CONTROL_CLIENT,"Connect to EMA Client successfully");
        return 1;
    }
}

void close_socket(int fd_sock)					//关闭socket连接
{
    closesocket(fd_sock);
    printmsg(ECU_DBG_OTHER,"Close socket");
}

int msg_is_complete(const char *s)
{
    int i, msg_length = 18;
    char buffer[6] = {'\0'};

    if(strlen(s) < 10)
        return 0;

    //从信息头获取信息长度
    strncpy(buffer, &s[5], 5);
    for(i=0; i<5; i++)
        if('A' == buffer[i])
            buffer[i] = '0';
    msg_length = atoi(buffer);

    //将实际收到的长度与信息中定义的长度作比较
    if(strlen(s) < msg_length){
        return 0;
    }
    return 1;
}



/* Socket 发送数据 */
int send_socket(int sockfd, char *sendbuffer, int size)
{
    int i, send_count;
    char msg_length[6] = {'\0'};
    int length = 0,send_length = 0;


    if(sendbuffer[strlen(sendbuffer)-1] == '\n'){
        sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
    }
    else{
        sprintf(msg_length, "%05d", strlen(sendbuffer));
        strcat(sendbuffer, "\n");
        size++;
    }
    length = size;
    strncpy(&sendbuffer[5], msg_length, 5);
    for(i=0; i<3; i++){

        while(length > 0)
        {
            if(length > SIZE_PER_SEND)
            {
                send_count = send(sockfd, &sendbuffer[send_length], SIZE_PER_SEND, 0);
                send_length += SIZE_PER_SEND;
                length -= SIZE_PER_SEND;
            }else
            {
                send_count = send(sockfd, &sendbuffer[send_length], length, 0);

                length -= length;
            }

            if(-1 == send_count)
            {
                close_socket(sockfd);
                return -1;
            }

            rt_hw_ms_delay(500);
        }

        if(send_count >= 0){
            sendbuffer[strlen(sendbuffer)-1] = '\0';
            print2msg(ECU_DBG_CONTROL_CLIENT,"Sent", sendbuffer);
            rt_hw_ms_delay(100);
            return send_count;
        }
    }
    printmsg(ECU_DBG_CONTROL_CLIENT,"Send failed:");
    return -1;
}

/* 接收数据 */
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s)
{
    fd_set rd;
    struct timeval timeout = {10,0};
    int recv_each = 0, recv_count = 0,res = 0;
    char *recv_buffer = NULL;
    recv_buffer = malloc(4096);

    memset(recvbuffer, '\0', size);
    while(1)
    {
        FD_ZERO(&rd);
        FD_SET(sockfd, &rd);
        timeout.tv_sec = timeout_s;
        res = select(sockfd+1, &rd, NULL, NULL, &timeout);
        switch(res){
        case -1:
            printmsg(ECU_DBG_CONTROL_CLIENT,"select");
        case 0:
            printmsg(ECU_DBG_CONTROL_CLIENT,"Receive date from EMA timeout");
            closesocket(sockfd);
            printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
            free(recv_buffer);
            recv_buffer = NULL;
            return -1;
        default:
            if(FD_ISSET(sockfd, &rd)){
                memset(recv_buffer, '\0', sizeof(recv_buffer));
                recv_each = recv(sockfd, recv_buffer, 4096, 0);
                recv_buffer[recv_each] = '\0';
                strcat(recvbuffer, recv_buffer);
                if(recv_each <= 0){
                    printdecmsg(ECU_DBG_CONTROL_CLIENT,"Communication over", recv_each);
                    free(recv_buffer);
                    recv_buffer = NULL;
                    return -1;
                }
                printdecmsg(ECU_DBG_CONTROL_CLIENT,"Received each time", recv_each);
                recv_count += recv_each;
                print2msg(ECU_DBG_CONTROL_CLIENT,"Received", recvbuffer);
                if(msg_is_complete(recvbuffer)){
                    free(recv_buffer);
                    recv_buffer = NULL;
                    return recv_count;
                }
            }
            break;
        }
    }

}


/* Socket客户端初始化 返回-1表示失败*/ 
int Control_client_socket_init(void)
{
    int sockfd;
    int ret;

    sockfd = createsocket();
    //创建SOCKET描述符
    if(sockfd < 0) return sockfd;
    ret = connect_control_socket(sockfd);
    //小于0  表示创建失败
    if(ret < 0) return ret;
    return sockfd;
}

int recv_response(int fd_sock, char *readbuff)
{
    fd_set rd;
    struct timeval timeout;
    int recvbytes, res, count=0, readbytes = 0;
    char *recvbuff = NULL;
    char temp[16];
    recvbuff = malloc((4+99*14));
    memset(recvbuff,'\0',(4+99*14));
    while(1)
    {
        FD_ZERO(&rd);
        FD_SET(fd_sock, &rd);
        timeout.tv_sec = 20;
        timeout.tv_usec = 0;

        res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
        if(res <= 0){
            //printerrmsg("select");
            printmsg(ECU_DBG_CLIENT,"Receive data reply from EMA timeout");
            free(recvbuff);
            recvbuff = NULL;
            return -1;
        }
        else{
            memset(recvbuff, '\0', sizeof(recvbuff));
            memset(temp, '\0', sizeof(temp));
            recvbytes = recv(fd_sock, recvbuff, sizeof(recvbuff), 0);
            if(0 == recvbytes)
            {
                free(recvbuff);
                recvbuff = NULL;
                return -1;
            }
            strcat(readbuff, recvbuff);
            readbytes += recvbytes;
            if(readbytes >= 3)
            {
                count = (readbuff[1]-0x30)*10 + (readbuff[2]-0x30);
                if(count==((strlen(readbuff)-3)/14))
                {
                    free(recvbuff);
                    recvbuff = NULL;
                    return readbytes;
                }
            }
        }
    }
}

//与Client服务器通讯 
//sendbuff[in]:发送数据的buff
//sendLength[in]:发送字节长度
//recvbuff[out]:接收数据buff
//recvLength[in/out]:in接收数据最大长度  out接收数据长度
//Timeout[in]	超时时间 ms
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout)
{
    int socketfd = 0;
    int readbytes = 0,count =0,recvlen =0;
    char *readbuff = NULL;
    int length = 0,send_length = 0;
    length = sendLength;
    socketfd = createsocket();
    if(socketfd == -1) 	//创建socket失败
    {
        LED_Status = 0;
        return -1;
    }

    //创建socket成功
    if(1 == connect_client_socket(socketfd))
    {	//连接服务器成功
        int sendbytes = 0;
        int res = 0;
        fd_set rd;
        struct timeval timeout;

        while(length > 0)
        {
            if(length > SIZE_PER_SEND)
            {
                sendbytes = send(socketfd, &sendbuff[send_length], SIZE_PER_SEND, 0);
                send_length += SIZE_PER_SEND;
                length -= SIZE_PER_SEND;
            }else
            {
                sendbytes = send(socketfd, &sendbuff[send_length], length, 0);

                length -= length;
            }

            if(-1 == sendbytes)
            {
                close_socket(socketfd);
                LED_Status = 0;
                return -1;
            }

            rt_hw_ms_delay(500);
        }



        readbuff = malloc((4+99*14));
        memset(readbuff,'\0',(4+99*14));

        while(1)
        {
            FD_ZERO(&rd);
            FD_SET(socketfd, &rd);
            timeout.tv_sec = Timeout/1000;
            timeout.tv_usec = Timeout%1000;

            res = select(socketfd+1, &rd, NULL, NULL, &timeout);

            if(res <= 0){
                printmsg(ECU_DBG_CLIENT,"Receive data reply from Server timeout");
                close_socket(socketfd);
                free(readbuff);
                readbuff = NULL;
                LED_Status = 0;
                return -1;
            }else
            {
                memset(readbuff, '\0', sizeof(readbuff));
                readbytes = recv(socketfd, readbuff, *recvLength, 0);
                if(readbytes <= 0)
                {
                    free(readbuff);
                    readbuff = NULL;
                    *recvLength = 0;
                    close_socket(socketfd);
                    LED_Status = 0;
                    return -1;
                }
                strcat(recvbuff,readbuff);
                recvlen += readbytes;
                if(recvlen >= 3)
                {
                    //print2msg(ECU_DBG_CLIENT,"recvbuff:",recvbuff);
                    *recvLength = recvlen;
                    if('{' == recvbuff[0])
                    {
                        free(readbuff);
                        readbuff = NULL;
                        close_socket(socketfd);
                        LED_Status = 1;
                        return *recvLength;
                    }


                    count = (recvbuff[1]-0x30)*10 + (recvbuff[2]-0x30);
                    if(count==((strlen(recvbuff)-3)/14))
                    {
                        free(readbuff);
                        readbuff = NULL;
                        close_socket(socketfd);
                        LED_Status = 1;
                        return *recvLength;
                    }else if(recvbuff[0] != '1')
                    {
                        free(readbuff);
                        readbuff = NULL;
                        close_socket(socketfd);
                        *recvLength = 0;
                        LED_Status = 0;
                        return *recvLength;
                    }
                }

            }
        }


    }else
    {
        //连接服务器失败
        //失败情况下通过无线发送
#ifdef WIFI_USE
        int ret = 0,i = 0;
        ret = SendToSocketB(client_arg.ip,randport(client_arg),sendbuff, sendLength);
        if(ret == -1)
        {
            LED_Status = 0;
            return -1;
        }

        for(i = 0;i<(Timeout/10);i++)
        {
            if(WIFI_Recv_SocketB_Event == 1)
            {
                *recvLength = WIFI_Recv_SocketB_LEN;
                memcpy(recvbuff,WIFI_RecvSocketBData,*recvLength);
                recvbuff[*recvLength] = '\0';
                //sprint2msg(ECU_DBG_CLIENT,"serverCommunication_Client",recvbuff);
                WIFI_Recv_SocketB_Event = 0;
                LED_Status = 1;
                return 0;
            }
            rt_thread_delay(1);
        }
        LED_Status = 0;
        return -1;
#endif

#ifndef WIFI_USE
        LED_Status = 0;
        return -1;
#endif
    }

}


//与Control服务器通讯 
//sendbuff[in]:发送数据的buff
//sendLength[in]:发送字节长度
//recvbuff[out]:接收数据buff
//recvLength[in/out]:in接收数据最大长度  out接收数据长度
//Timeout[in]	超时时间 ms
int serverCommunication_Control(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout)
{
    int socketfd = 0;
    int length = 0,send_length = 0;
    socketfd = createsocket();
    length = sendLength;

    if(socketfd == -1) 	//创建socket失败
        return -1;
    //创建socket成功
    if(1 == connect_control_socket(socketfd))
    {	//连接服务器成功
        int sendbytes = 0;
        int res = 0;
        fd_set rd;
        struct timeval timeout;
        while(length > 0)
        {
            if(length > SIZE_PER_SEND)
            {
                sendbytes = send(socketfd, &sendbuff[send_length], SIZE_PER_SEND, 0);
                send_length += SIZE_PER_SEND;
                length -= SIZE_PER_SEND;
            }else
            {
                sendbytes = send(socketfd, &sendbuff[send_length], length, 0);

                length -= length;
            }

            if(-1 == sendbytes)
            {
                close_socket(socketfd);
                return -1;
            }

            rt_hw_ms_delay(500);
        }

        FD_ZERO(&rd);
        FD_SET(socketfd, &rd);
        timeout.tv_sec = Timeout/1000;
        timeout.tv_usec = Timeout%1000;
        res = select(socketfd+1, &rd, NULL, NULL, &timeout);
        if(res <= 0){
            printmsg(ECU_DBG_CONTROL_CLIENT,"Receive data reply from Server timeout");
            close_socket(socketfd);
            return -1;
        }else
        {
            *recvLength = recv(socketfd, recvbuff, *recvLength, 0);
            if(0 == *recvLength)
            {
                close_socket(socketfd);
                return -1;
            }else
            {
                print2msg(ECU_DBG_CONTROL_CLIENT,"serverCommunication_Control:",recvbuff);
                close_socket(socketfd);
                return 0;
            }
        }

    }else
    {
        //连接服务器失败
        //失败情况下通过无线发送
#ifdef WIFI_USE
        int ret = 0,i = 0;
        ret = SendToSocketC(control_client_arg.ip,randport(control_client_arg),sendbuff, sendLength);
        if(ret == -1)
            return -1;
        for(i = 0;i<(Timeout/10);i++)
        {
            if(WIFI_Recv_SocketC_Event == 1)
            {
                *recvLength = WIFI_Recv_SocketC_LEN;
                memcpy(recvbuff,WIFI_RecvSocketCData,*recvLength);
                recvbuff[*recvLength] = '\0';
                //print2msg(ECU_DBG_CONTROL_CLIENT,"serverCommunication_Control:",recvbuff);
                WIFI_Recv_SocketC_Event = 0;
                //WIFI_Close(SOCKET_C);
                return 0;
            }
            rt_thread_delay(1);
        }
        //WIFI_Close(SOCKET_C);
        return -1;
#endif

#ifndef WIFI_USE
        return -1;
#endif
    }

}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(initSocketArgs, eg:initSocketArgs());
#endif
