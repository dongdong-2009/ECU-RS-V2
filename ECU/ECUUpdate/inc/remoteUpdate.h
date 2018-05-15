#ifndef __REMOTE_UPDATE_H__
#define __REMOTE_UPDATE_H__
/*****************************************************************************/
/* File      : remoteUpdate.h                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-11 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
void remote_update_thread_entry(void* parameter);
int updateECUByTrinaSolar(char *domain,char *host, int port, char *user, char *pwd,char *remotefile);
int updateECUByID_Local(char *Domain,char *IP,int port,char *User,char *passwd);
int updateECUByVersion_Local(char *Domain,char *IP,int port,char *User,char *passwd);


#endif /*__REMOTE_UPDATE_H__*/
