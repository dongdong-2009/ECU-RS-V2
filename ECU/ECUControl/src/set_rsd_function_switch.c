#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "rthw.h"
#include "threadlist.h"
#include "rtthread.h"
#include "version.h"
#include "set_rsd_function_switch.h"
#include "debug.h"
#include "serverfile.h"
#include "rsdFunction.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//A160����RSD���ܿ���
int set_rsd_function_switch(const char *recvbuffer, char *sendbuffer)
{
	int flag;
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	char strfunstatus[2] = {'\0'};
	char stronoff[2] = {'\0'};
	char strrsdTimeout[4] = {'\0'};
	unsigned char FunctionStatus,onoffstatus,rsdTimeout;
	char strnum[5] = {'\0'};
	unsigned short num = 0;
	sscanf(&recvbuffer[30], "%1d", &flag);

	//��ȡʱ���
	strncpy(timestamp, &recvbuffer[31], 14);
	timestamp[14] = '\0';
	printf("recvbuffer:%s\n",recvbuffer);
	printf("flag:%d\n",flag);
	printf("timestamp:%s\n",timestamp);
	

	//���ݿ�򿪳ɹ����������ݲ���
	switch(flag)
	{
		case 0:	//�㲥����
			memcpy(strfunstatus,&recvbuffer[48],1);
			memcpy(stronoff,&recvbuffer[49],1);
			memcpy(strrsdTimeout,&recvbuffer[50],3);
			FunctionStatus = atoi(strfunstatus);
			onoffstatus = atoi(stronoff);
			rsdTimeout = atoi(strrsdTimeout);
			saveChangeFunctionStatus(FunctionStatus,onoffstatus,rsdTimeout);
			save_rsdFunction_change_flag();
			//����main�߳�
			threadRestartTimer(20,TYPE_DATACOLLECT);

			break;
		case 1:	//��������
			memcpy(strnum,&recvbuffer[48],4);
			num = atoi(strnum);
			insertSetRSDInfo(num,(char *)&recvbuffer[52]);
			//����main�߳�
			threadRestartTimer(20,TYPE_DATACOLLECT);
			
			break;
		default:
			ack_flag = FORMAT_ERROR; //��ʽ����
			break;
	}

	//ƴ��Ӧ����Ϣ
	msg_ACK(sendbuffer, "A160", timestamp, ack_flag);
	return 0; //������һ��ִ������������
}
