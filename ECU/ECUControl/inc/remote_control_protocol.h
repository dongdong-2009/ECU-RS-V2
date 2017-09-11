#ifndef __REMOTE_CONTROL_PROTOCOL_H__
#define __REMOTE_CONTROL_PROTOCOL_H__
/*****************************************************************************/
/* File      : remote_control_protocol.h                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
/* 出错信息定义 */
#define SUCCESS 0
#define FORMAT_ERROR 1
#define UNSUPPORTED 2
#define DB_ERROR 3
#define CMD_ERROR 4
#define FILE_ERROR 5

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

int msg_Header(char *sendbuffer, const char *cmd_id);
int msg_REQ(char *sendbuffer);
int msg_ACK(char *sendbuffer, const char *cmd_id, const char *timestamp, int ack_flag);

int msg_get_int(const char *s, int len);
int msg_version(const char *msg);
int msg_length(const char *msg);
int msg_cmd_id(const char *msg);
int msg_seq_id(const char *msg);

int msg_format_check(const char *msg);
int msg_num_check(const char *s, int num, int len, int flag);

char *msgcat_s(char *s, int size, const char *value);
char *msgcat_d(char *s, int size, int value);
int msg_get_one_section(char *s, const char *msg);

#endif	/*__REMOTE_CONTROL_PROTOCOL_H__*/
