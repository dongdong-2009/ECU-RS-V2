#include <stdio.h>
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
	
	//��ȡ�������ͱ�־λ: 0��������; 1��������; 2ɾ�������
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
		case 0:
			//RSD���ܹر�
			printmsg(ECU_DBG_CONTROL_CLIENT,"Write_IO_INIT_STATU(0)");
			
			saveChangeFunctionStatus('0');
			save_rsdFunction_change_flag();
			//����main�߳�
			restartThread(TYPE_COMM);

			break;
		case 1:
			//RSD���ܴ�
			printmsg(ECU_DBG_CONTROL_CLIENT,"Write_IO_INIT_STATU(1)");
			
			saveChangeFunctionStatus('1');
			save_rsdFunction_change_flag();
			//����main�߳�
			restartThread(TYPE_COMM);
			

			break;
		default:
			ack_flag = FORMAT_ERROR; //��ʽ����
			break;
	}

	//ƴ��Ӧ����Ϣ
	msg_ACK(sendbuffer, "A160", timestamp, ack_flag);
	return 0; //������һ��ִ������������
}
