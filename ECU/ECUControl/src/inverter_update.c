/*****************************************************************************/
/* File      : inverter_update.c                                             */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-04 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "Serverfile.h"
#include "rtthread.h"
#include "mycommand.h"

/*********************************************************************
upinv����ֶΣ�
id,update_result,update_time,update_flag
**********************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* ����ָ��̨���������������־ */
int set_update_num(const char *msg, int num)
{

	int i, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100] = {'\0'};
	
	for(i=0; i<num; i++)
	{
		//��ȡһ̨�������ID��
		strncpy(inverter_id, &msg[i*13], 12);
			
		//������ڸ������������ɾ���ü�¼
		delete_line("/home/data/upinv","/home/data/upinv.t",inverter_id,12);
		sprintf(str,"%s,,,1\n",inverter_id);
		//��������
		if(-1 == insert_line("/home/data/upinv",str))
		{
			err_count++;
		}	
	}

	return err_count;
}

/* ��A136��EMAԶ����������� */
int set_inverter_update(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//��ȡ�������ͱ�־λ: 0���������������ע��Ŀǰû�л�ԭ����������ܣ���1����ָ�������
	type = msg_get_int(&recvbuffer[30], 1);
	//��ȡ���������
	num = msg_get_int(&recvbuffer[31], 4);
	//��ȡʱ���
	strncpy(timestamp, &recvbuffer[35], 14);

	switch(type)
	{
		case 0:
			//�����������������
			ack_flag = UNSUPPORTED;
			break;
		case 1:
			//����ʽ�������������
			if(!msg_num_check(&recvbuffer[52], num, 13, 0)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				//����ָ����������������ݿ�
				if(set_update_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;

				reboot_timer(10);
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	//ƴ��Ӧ����Ϣ
	msg_ACK(sendbuffer, "A136", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
