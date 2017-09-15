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
/* A108 EMA ��ECU�����Զ������� */
int custom_command(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char command[256] = {'\0'};
	char timestamp[15] = {'\0'};

	//��ȡ����ʱ���
	strncpy(timestamp, &recvbuffer[30], 14);

	//�Զ�������
	if(msg_get_one_section(command, &recvbuffer[47]) <= 0){
		ack_flag = FORMAT_ERROR;
	}
	else{
		//������������
		if(!strncmp(command, "quit", 4)){
			printmsg(ECU_DBG_CONTROL_CLIENT,"Ready to quit");
			msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
			return -1;
		}
		//ִ���Զ�������
		ack_flag = mysystem(command);
	}
	msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
	return 0;
}
