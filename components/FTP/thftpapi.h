#ifndef __THFTPAPI_H__
#define __THFTPAPI_H__
/*****************************************************************************/
/* File      : thftpapi.h                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

//获取连接服务器IP
void getFTPConf(char *FTPIP,int *port,char* user,char *password);
//连接服务器  OK
int ftp_connect( char *host, int port, char *user, char *pwd );
//断开服务器  OK
int ftp_quit( int c_sock);
//设置表示类型 OK
int ftp_type( int c_sock, char mode );
//删除文件
int ftp_deletefile( int c_sock, char *s );
//下载文件
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//上传文件
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//下载文件
int ftpgetfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
//上传文件
int ftpputfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
//上传文件
int putfile(char *remoteFile, char *localFile);
//下载文件
int getfile(char *remoteFile, char *localFile);
//删除文件
int deletefile(char *remoteFile);


#endif /*__THFTPAPI_H__*/
