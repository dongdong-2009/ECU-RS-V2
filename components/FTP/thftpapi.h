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

//��ȡ���ӷ�����IP
void getFTPConf(char *domain,char *FTPIP,int *port,char* user,char *password);

//���ӷ�����  OK
int ftp_connect(char *domain, char *host, int port, char *user, char *pwd );
//�Ͽ�������  OK
int ftp_quit( int c_sock);
//���ñ�ʾ���� OK
int ftp_type( int c_sock, char mode );
//ɾ���ļ�
int ftp_deletefile( int c_sock, char *s );
//�����ļ�
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//�ϴ��ļ�
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//�����ļ�
int ftpgetfile(char *domain,char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
//�ϴ��ļ�
int ftpputfile(char *domain,char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
//�ϴ��ļ�
int putfile(char *remoteFile, char *localFile);
//�����ļ�
int getfile(char *remoteFile, char *localFile);
//ɾ���ļ�
int deletefile(char *remoteFile);
int ftpgetfile_InternalFlash(char *domain,char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);


#endif /*__THFTPAPI_H__*/
