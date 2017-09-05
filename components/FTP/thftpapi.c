/*****************************************************************************/
/* File      : thftpapi.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
 
 /*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "thftpapi.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"
#include "datetime.h"
#include "debug.h"
#include "lan8720rst.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
void getFTPConf(char *FTPIP,int *port,char* user,char *password)
{
	FILE *fp;
	char buff[50];
	fp = fopen("/yuneng/ftpadd.con", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(!strlen(buff))
				break;
			if(!strncmp(buff, "IP", 2))
			{
				strcpy(FTPIP, &buff[3]);
				if('\n' == FTPIP[strlen(FTPIP)-1])
					FTPIP[strlen(FTPIP)-1] = '\0';
			}
			if(!strncmp(buff, "Port", 4))
				*port = atoi(&buff[5]);
			if(!strncmp(buff, "user", 4))
			{
				strcpy(user, &buff[5]);
				if('\n' == user[strlen(user)-1])
					user[strlen(user)-1] = '\0';				
			}
			if(!strncmp(buff, "password", 8))
			{
				strcpy(password, &buff[9]);
				if('\n' == password[strlen(password)-1])
					password[strlen(password)-1] = '\0';				
			}
		}
		fclose(fp);
	}else
	{
		strcpy(FTPIP,"60.190.131.190");
		*port = 9219;
		strcpy(user,"zhyf");
		strcpy(password,"yuneng");
	}
	
}

 
//创建一个socket并返回
int socket_connect(char *host,int port)
{
    struct sockaddr_in address;
    int s, opvalue;
    socklen_t slen;
    //设置接收和发送超时
    struct timeval timeo = {15, 0}; 
		struct hostent* server;
    opvalue = 8;
    slen = sizeof(opvalue);
    memset(&address, 0, sizeof(address));
     
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        setsockopt(s, IPPROTO_IP, IP_TOS, &opvalue, slen) < 0)
        return -1;
     
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
     
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
     
    server = gethostbyname(host);
    if (!server)
    {
    	closesocket(s);
     	return -1;
    }
	
    memcpy(&address.sin_addr.s_addr, server->h_addr, server->h_length); 
    if (connect(s, (struct sockaddr*) &address, sizeof(address)) == -1)
    {
    	closesocket(s);
    	return -1;
    }  
     
    return s;
}
 
//连接到一个ftp的服务器，返回socket
int connect_server( char *host, int port )
{   
    int       ctrl_sock;
    char      *buf;
    int       result;
    ssize_t   len;
    fd_set rd;
	struct timeval timeout;
	buf = malloc(512);
    ctrl_sock = socket_connect(host, port);
    if (ctrl_sock == -1) {
		free(buf);
        return -1;
    }
		FD_ZERO(&rd);
		FD_SET(ctrl_sock, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		len = select(ctrl_sock+1, &rd, NULL, NULL, &timeout);
		if(len <= 0){
			closesocket( ctrl_sock );
			free(buf);
			return -1;
		}else
		{
			memset(buf,0x00,512);
			len = recv( ctrl_sock, buf, 512, 0 );
			buf[len] = 0;
			sscanf( buf, "%d", &result );
			if ( result != 220 ) {
					closesocket( ctrl_sock );
					free(buf);
					return -1;
			}
			free(buf);
			return ctrl_sock;
		}
}
 
//发送命令,返回结果
int ftp_sendcmd_re( int sock, char *cmd, void *re_buf, ssize_t *len)
{
    char        *buf;
    ssize_t     r_len;
    fd_set rd;
		struct timeval timeout;
	
	buf = malloc(512);
    if ( send( sock, cmd, strlen(cmd), 0 ) == -1 )
		{
			free(buf);
      return -1;
		}
		
		FD_ZERO(&rd);
		FD_SET(sock, &rd);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		r_len = select(sock+1, &rd, NULL, NULL, &timeout);
		if(len <= 0){
			free(buf);
			return -1;
		}else
		{
			r_len = recv( sock, buf, 512, 0 );
			if ( r_len < 1 ) 
			{
				free(buf);
				return -1;
			}
			buf[r_len] = 0;
			 
			if (len != NULL) *len = r_len;
			if (re_buf != NULL) sprintf(re_buf, "%s", buf);
			free(buf);
			return 0;
		}
}
 
//发送命令,返回编号
int ftp_sendcmd( int sock, char *cmd )
{
    char     *buf;
    int      result;
    ssize_t  len;

	buf = malloc(512);
    result = ftp_sendcmd_re(sock, cmd, buf, &len);
    if (result == 0)
    {
        sscanf( buf, "%d", &result );
    }
    free(buf); 
    return result;
}
 
//登录ftp服务器
int login_server( int sock, char *user, char *pwd )
{
    char    *buf;
    int     result;

	buf = malloc(128); 
    sprintf( buf, "USER %s\r\n", user );
    result = ftp_sendcmd( sock, buf );
    if ( result == 230 )
	{
		free(buf);
		return 0;
	}
    else if ( result == 331 ) {
        sprintf( buf, "PASS %s\r\n", pwd );
        if ( ftp_sendcmd( sock, buf ) != 230 ) 
		{
			free(buf);
			return -1;
		}
		free(buf);
        return 0;
    }
    else
    {
    	free(buf);
    	return -1;
    }
        
}
 
int create_datasock( int ctrl_sock )
{
    int     lsn_sock;
    int     port;
    int     len;
    struct sockaddr_in sin;
    char    cmd[128];
     
    lsn_sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( lsn_sock == -1 ) return -1;
    memset( (char *)&sin, 0, sizeof(sin) );
    sin.sin_family = AF_INET;
    if( bind(lsn_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1 ) {
        closesocket( lsn_sock );
        return -1;
    }
     
    if( listen(lsn_sock, 2) == -1 ) {
        closesocket( lsn_sock );
        return -1;
    }
     
    len = sizeof( struct sockaddr );
    if ( getsockname( lsn_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        closesocket( lsn_sock );
        return -1;
    }
    port = sin.sin_port;
     
    if( getsockname( ctrl_sock, (struct sockaddr *)&sin, (socklen_t *)&len ) == -1 )
    {
        closesocket( lsn_sock );
        return -1;
    }
     
    sprintf( cmd, "PORT %d,%d,%d,%d,%d,%d\r\n",
            sin.sin_addr.s_addr&0x000000FF,
            (sin.sin_addr.s_addr&0x0000FF00)>>8,
            (sin.sin_addr.s_addr&0x00FF0000)>>16,
            (sin.sin_addr.s_addr&0xFF000000)>>24,
            port>>8, port&0xff );
     
    if ( ftp_sendcmd( ctrl_sock, cmd ) != 200 ) {
        closesocket( lsn_sock );
        return -1;
    }
    return lsn_sock;
}
 
//连接到PASV接口
int ftp_pasv_connect( int c_sock )
{
    int     r_sock;
    int     send_re;
    ssize_t len;
    int     addr[6];
    char    buf[512];
    char    re_buf[512];
     
    //设置PASV被动模式
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "PASV\r\n");
    send_re = ftp_sendcmd_re( c_sock, buf, re_buf, &len);
    if (send_re == 0) {
        sscanf(re_buf, "%*[^(](%d,%d,%d,%d,%d,%d)",&addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
    }
     
    //连接PASV端口
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
		print2msg(ECU_DBG_UPDATE,"UPDATE IP",buf);
		printdecmsg(ECU_DBG_UPDATE,"UPDATE PORT",(addr[4]*256+addr[5]));
    r_sock = socket_connect(buf,addr[4]*256+addr[5]);
     
    return r_sock;
}
 
//表示类型
int ftp_type( int c_sock, char mode )
{
    char    buf[128];
    sprintf( buf, "TYPE %c\r\n", mode );
    if ( ftp_sendcmd( c_sock, buf ) != 200 )
        return -1;
    else
        return 0;
}
 
//下载文件
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock = 0;
    ssize_t len,write_len,sum=0;
    char    *buf;
    int     handle;
    int     result =0;
    fd_set rd;
		struct timeval timeout;	
	  //char time[20];
	 buf = malloc(1461);
    //打开本地文件
    handle = fileopen( d,  O_WRONLY | O_CREAT | O_TRUNC, 0 );
    if ( handle == -1 ) 
		{
			free(buf);
			printmsg(ECU_DBG_UPDATE,"ftp_retrfile FILE open failed");
			unlink(d);
			return -1;
		}
    //设置传输模式
    ftp_type(c_sock, 'I');
     
    //连接到PASV接口
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
			printmsg(ECU_DBG_UPDATE,"ftp_pasv_connect failed");
      fileclose(handle);
			free(buf);
			unlink(d);
       return -1;
    }
     
    //发送STOR命令
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "RETR %s\r\n", s );
    result = ftp_sendcmd( c_sock, buf );
    if (result >= 300 || result == 0)
    {
    	closesocket( d_sock );
		printmsg(ECU_DBG_UPDATE,"RETR response error");
	    fileclose(handle);
		free(buf);
		unlink(d);
      return result;
    }
     
    //开始向PASV读取数据
    memset(buf,0x00, sizeof(buf));
		FD_ZERO(&rd);
		FD_SET(d_sock, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		while (1) {
			len = select(d_sock+1, &rd, NULL, NULL, &timeout);
			if(len <= 0){
				printmsg(ECU_DBG_UPDATE,"ftp_retrfile select out");
				break;
			}else
			{
				if((len = recv( d_sock, buf, 1460, MSG_DONTWAIT )) > 0 )
				{
					sum += len;

					write_len = fileWrite( handle, buf, len );
					if (write_len != len || (stop != NULL && *stop))
					{
							closesocket( d_sock );
							fileclose( handle );
							free(buf);
							unlink(d);
							return -1;
					}
					 
					if (stor_size != NULL)
					{
							*stor_size += write_len;
					}
					memset(buf,0x00, sizeof(buf));
					if(len < 1){	

						printdecmsg(ECU_DBG_UPDATE,"transfer",len);
						break;
					}
				}
				else
				{
					break;
				}
			}				
		}
		closesocket( d_sock );
		fileclose( handle );
		

		timeout.tv_sec = 3;
		timeout.tv_usec = 0;		
    //向服务器接收返回值
    memset(buf,0x00, sizeof(buf));
		len = recv( c_sock, buf, 512, 0 );
		if(len <= 0)
		{
			free(buf);
			unlink(d);
			return -1;
		}else
		{
			
			buf[len] = 0;
			sscanf( buf, "%d", &result );
			if ( result >= 300 ) {
				free(buf);
				unlink(d);
        return result;
			}
			free(buf);
			return 0;
		}
		
}

//上传文件
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop)
{
    int     d_sock;
    ssize_t     len,send_len;
    char    *buf;
    int     handle;
    int send_re;
    int result = 0;

		buf = malloc(512);
    //打开本地文件
    handle = fileopen( s,  O_RDONLY,0);
    if ( handle == -1 ) 
		{
			free(buf);
			return -1;
		}
     
    //设置传输模式
    ftp_type(c_sock, 'I');
     
    //连接到PASV接口
    d_sock = ftp_pasv_connect(c_sock);
    if (d_sock == -1)
    {
      fileclose(handle);
			free(buf);
      return -1;
    }

    //发送STOR命令
    memset(buf,0x00, sizeof(buf));
    sprintf( buf, "STOR %s\r\n", d );
    send_re = ftp_sendcmd( c_sock, buf );
    if (send_re >= 300 || send_re == 0)
    {
      fileclose(handle);
			free(buf);
      return send_re;
    }
     
    //开始向PASV通道写数据
    memset(buf,0x00, sizeof(buf));
    while ( (len = fileRead( handle, buf, 512)) > 0)
    {
        send_len = send(d_sock, buf, len, 0);
			
        if (send_len != len ||
            (stop != NULL && *stop))
        {
					printdecmsg(ECU_DBG_UPDATE,"send_len ",send_len);
					printdecmsg(ECU_DBG_UPDATE,"len:%d ",len);
          closesocket( d_sock );
          fileclose( handle );
					free(buf);
          return -1;
        }
         
        if (stor_size != NULL)
        {
            *stor_size += send_len;
        }
    }
    closesocket( d_sock );
    fileclose( handle );
     
    //向服务器接收返回值
    memset(buf,0x00, sizeof(buf));
    len = recv( c_sock, buf, 512, 0 );
    buf[len] = 0;
    sscanf( buf, "%d", &result );
    if ( result >= 300 ) {
			free(buf);
      return result;
    }
		free(buf);
    return 0;
}
 
//连接服务器
int ftp_connect( char *host, int port, char *user, char *pwd )
{
    int     c_sock;
	if(rt_hw_GetWiredNetConnect() == 0)
	{
		return -1;
	}
    c_sock = connect_server( host, port );
    if ( c_sock == -1 ) return -1;
    if ( login_server( c_sock, user, pwd ) == -1 ) {
        closesocket( c_sock );
        return -1;
    }
    return c_sock;
}
 
//断开服务器
int ftp_quit( int c_sock)
{
    int re = 0;
    re = ftp_sendcmd( c_sock, "QUIT\r\n" );
    closesocket( c_sock );
    return re;
}

//删除文件
int ftp_deletefile( int c_sock, char *s )
{
    char    buf[512];
    int     re;
     
    sprintf( buf, "DELE %s\r\n", s );
    re = ftp_sendcmd( c_sock, buf );
    if ( re != 250 ) return re;
    return 0;
}


//下载文件
int ftpgetfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile)
{
	unsigned long long stor_size = 0;
	int stop = 0,ret = 0;
	int sockfd = ftp_connect( host, port, user, pwd  );
	if(sockfd != -1)
	{
		printdecmsg(ECU_DBG_UPDATE,"ftp connect successful",sockfd);
	}else
	{
		printmsg(ECU_DBG_UPDATE,"ftp connect failed");
		return -1;
	}
	
	ret = ftp_retrfile(sockfd, remotefile, localfile ,&stor_size, &stop);
	printdecmsg(ECU_DBG_UPDATE,"ret",ret);
	printdecmsg(ECU_DBG_UPDATE,"stor_size",stor_size);
	printdecmsg(ECU_DBG_UPDATE,"stop",stop);
	ftp_quit( sockfd);
	return ret;
}

//上传文件
int ftpputfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile)
{
	unsigned long long stor_size = 0;
	int stop = 0,ret = 0;
	int sockfd = ftp_connect( host, port, user, pwd  );

	if(sockfd != -1)
	{
		printdecmsg(ECU_DBG_UPDATE,"ftp connect successful",sockfd);	
	}else
	{
		printmsg(ECU_DBG_UPDATE,"ftp connect failed");
		return -1;
	}

  	ret = ftp_storfile(sockfd, localfile,remotefile ,&stor_size, &stop);
	printdecmsg(ECU_DBG_UPDATE,"ret",ret);
	printdecmsg(ECU_DBG_UPDATE,"stor_size",stor_size);
	printdecmsg(ECU_DBG_UPDATE,"stop",stop);

	ftp_quit( sockfd);
	return ret;
}

//删除文件
int ftpdeletefile(char *host, int port, char *user, char *pwd,char *remotefile)
{
	int ret = 0;
	int sockfd = ftp_connect( host, port, user, pwd  );

	if(sockfd != -1)
	{
		printdecmsg(ECU_DBG_UPDATE,"ftp connect successful",sockfd);	
	}else
	{
		printmsg(ECU_DBG_UPDATE,"ftp connect failed");
		return -1;
	}
		
  	ret = ftp_deletefile( sockfd,remotefile);
	printdecmsg(ECU_DBG_UPDATE,"ftp_deletefile ret",ret);
	ftp_quit( sockfd);
	return ret;
}



int getfile(char *remoteFile, char *localFile)
{
	char FTPIP[50];
	int port=0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	getFTPConf(FTPIP,&port,user,password);
	
	print2msg(ECU_DBG_UPDATE,"FTPIP",FTPIP);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);

	return ftpgetfile(FTPIP,port, user, password,remoteFile,localFile);
}

int putfile(char *remoteFile, char *localFile)
{
	char FTPIP[50];
	int port=0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	getFTPConf(FTPIP,&port,user,password);

	print2msg(ECU_DBG_UPDATE,"FTPIP",FTPIP);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);

	return ftpputfile(FTPIP,port, user, password,remoteFile,localFile);
}

int deletefile(char *remoteFile)
{
	char FTPIP[50];
	int port=0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	getFTPConf(FTPIP,&port,user,password);

	print2msg(ECU_DBG_UPDATE,"FTPIP",FTPIP);
	printdecmsg(ECU_DBG_UPDATE,"port",port);
	print2msg(ECU_DBG_UPDATE,"user",user);
	print2msg(ECU_DBG_UPDATE,"password",password);

	return ftpdeletefile(FTPIP,port, user, password,remoteFile);
}


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(getfile,get file from ftp.)

FINSH_FUNCTION_EXPORT(putfile,put file from ftp.)

FINSH_FUNCTION_EXPORT(deletefile,delete file from ftp.)


#endif

 
