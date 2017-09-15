/*****************************************************************************/
/* File      : inverter_id.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-03 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "rthw.h"
#include "file.h"
#include "threadlist.h"
#include "rtthread.h"
#include "version.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 协议的ECU部分 */
int ecu_msg(char *sendbuffer, int num, const char *recvbuffer)
{


	char ecuid[13] = {'\0'};		//ECU号码
	char version_msg[16] = {'\0'};	//版本信息（包括：长度+版本号+数字版本号）
	char version[16] = {'\0'};		//版本号
	char area[16] = {'\0'};
	char version_number[2] = {'\0'};//数字版本号
	char timestamp[16] = {'\0'};	//时间戳

	/* 处理数据 */
	file_get_one(ecuid, sizeof(ecuid),
			"/yuneng/ecuid.con");
	sprintf(version,"%s_%s.%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	file_get_one(version_number, sizeof(version_number),
			"/yuneng/vernum.con");
	file_get_one(area, sizeof(area),
			"/yuneng/area.con");

	if(strlen(version_number)){
		sprintf(version_msg, "%02d%s%s--%s",
				strlen(version) + strlen(area) + 2 + strlen(version_number),
				version,
				area,
				version_number);
	}
	else{
		sprintf(version_msg, "%02d%s%s", strlen(version), version, area);
	}
	strncpy(timestamp, &recvbuffer[34], 14);

	/* 拼接ECU信息 */
	msgcat_s(sendbuffer, 12, ecuid);
	strcat(sendbuffer, version_msg);
	msgcat_d(sendbuffer, 3, num);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}

/* 协议的逆变器部分 */
int inverter_msg(char *sendbuffer, char* id)
{
	//添加逆变器ID
	strcat(sendbuffer, id); //逆变器ID
	strcat(sendbuffer, "00"); 	 //逆变器类型
	strcat(sendbuffer, "00000"); //逆变器版本号
	strcat(sendbuffer, "END"); 	 //结束符

	return 0;
}

/* 添加逆变器（返回添加成功的台数） */
int add_id(const char *msg, int num)
{

	int i, count = 0;
	char inverter_id[13] = {'\0'};

	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*15], 12);
		inverter_id[12] = '\0';
		//查询该逆变器ID在表中是否存在(建议建表的时候以‘id’为主键，方便使用REPLASE插入)
		//插入一个逆变器ID
		addInverter(inverter_id);
			count++;
	}

	return count;
}

/* 删除逆变器（返回删除成功的台数） */
int delete_id(const char *msg, int num)
{
	int i, count = 0;
	char inverter_id[13] = {'\0'};

	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*15], 12);
		inverter_id[12] = '\0';
		//删除一个逆变器ID
		delete_line("/home/data/id","/home/data/idtmp",inverter_id,12);
		count++;
	}
	return count;
}

/* 清空逆变器 */
int clear_id()
{
	return clear_file("/home/data/id");
}


/* 【A102】ECU上报逆变器ID */
int response_inverter_id(const char *recvbuffer, char *sendbuffer)
{
	//记录逆变器数量
	int num = 0,i;
	char inverter_ids[MAXINVERTERCOUNT][13];
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	/* Head */
	strcpy(sendbuffer, "APS13AAAAAA102AAA0"); //交给协议函数

	{
		//逆变器数量
		num = get_num_from_id(inverter_ids);
		/* ECU Message */
		ecu_msg(sendbuffer, num, recvbuffer);

		for(i = 0; i < num;i++)
		{
			if(12 == strlen(inverter_ids[i]))
			{
				/* Inverter Message */
				inverter_msg(sendbuffer,inverter_ids[i]);		
			}
		}
		
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

/* 【A103】EMA设置逆变器ID */
int set_inverter_id(const char *recvbuffer, char *sendbuffer)
{
	int flag, num;
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	
	//获取设置类型标志位: 0清除逆变器; 1添加逆变器; 2删除逆变器
	sscanf(&recvbuffer[30], "%1d", &flag);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 3);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);

	//检查格式
	if(!msg_num_check(&recvbuffer[51], num, 12, 1))
	{
		ack_flag = FORMAT_ERROR;
	}
	else
	{
		{
			//数据库打开成功，进行数据操作
			switch(flag)
			{
				case 0:
					//清空逆变器
					if(clear_id())
						ack_flag = DB_ERROR;
					break;
				case 1:
					//添加逆变器
					if(add_id(&recvbuffer[51], num) < num)
						ack_flag = DB_ERROR;
					break;
				case 2:
					//删除逆变器
					if(delete_id(&recvbuffer[51], num) < num)
						ack_flag = DB_ERROR;
					break;
				default:
					ack_flag = FORMAT_ERROR; //格式错误
					break;
			}
		}
		//重启主线程
		restartThread(TYPE_MAIN);
	}

	//拼接应答消息
	msg_ACK(sendbuffer, "A103", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 102; //返回下一个执行命令的命令号
}
