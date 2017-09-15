/*****************************************************************************/
/*  File      : custom_command.c                                             */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-07 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "mycommand.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 【A108】EMA向ECU发送自定义命令 */
int custom_command(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char command[256] = {'\0'};
	char timestamp[15] = {'\0'};

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);

	//自定义命令
	if(msg_get_one_section(command, &recvbuffer[47]) <= 0){
		ack_flag = FORMAT_ERROR;
	}
	else{
		//结束程序命令
		if(!strncmp(command, "quit", 4)){
			printmsg(ECU_DBG_CONTROL_CLIENT,"Ready to quit");
			msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
			return -1;
		}
		//执行自定义命令
		ack_flag = mysystem(command);
	}
	msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
	return 0;
}
