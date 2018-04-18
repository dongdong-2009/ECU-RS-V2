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
#include "threadlist.h"
#include "dfs_posix.h"

/*********************************************************************
upinv表格字段：
id,update_result,update_time,update_flag
**********************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 设置指定台数逆变器的升级标志 */
int set_update_num(const char *msg, int num)
{

	int i, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100] = {'\0'};
	int fd;
	char updateNum[2] = {'\0'};
	fd = open("/home/data/upinv", O_WRONLY  | O_TRUNC | O_CREAT,0);
	if (fd >= 0)
	{		
		for(i=0; i<num; i++)
		{
			//获取一台逆变器的ID号
			strncpy(inverter_id, &msg[i*13], 12);
			updateNum[0] = msg[i*13 + 12];
			updateNum[1] = '\0';
			sprintf(str,"%s,,,%s\n",inverter_id,updateNum);
			write(fd,str,strlen(str));
		}

		
		close(fd);
	}
	
	return err_count;
}

/* 【A136】EMA远程升级逆变器 */
int set_inverter_update(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取设置类型标志位: 0升级所有逆变器（注：目前没有还原所有这个功能），1升级指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	switch(type)
	{
		case 0:
			//不能升级所有逆变器
			ack_flag = UNSUPPORTED;
			break;
		case 1:
			//检查格式（逆变器数量）
			if(!msg_num_check(&recvbuffer[52], num, 13, 0)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				//升级指定逆变器，存入数据库
				if(set_update_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;

				//reboot_timer(10);
				restartThread(TYPE_UPDATE);
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A136", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
