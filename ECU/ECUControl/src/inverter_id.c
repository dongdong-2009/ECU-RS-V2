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
#include "threadlist.h"
#include "rtthread.h"
#include "version.h"
#include "inverter.h"
#include "dfs_posix.h"
#include "serverfile.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/*ECU HEAD Info*/
int ecu_msg(char *sendbuffer, int num, const char *recvbuffer)
{
	char ecuid[13] = {'\0'};		//ECU ID
	char version_msg[16] = {'\0'};	//版本信息
	char version[16] = {'\0'};		//版本号
	char area[16] = {'\0'};
	char version_number[2] = {'\0'};//数字版本号
	char timestamp[16] = {'\0'};	//时间戳

	/* 处理数据 */
	memcpy(ecuid,ecu.ECUID12,12);
	ecuid[12] = '\0';
	sprintf(version,"%s_%s.%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	memcpy(version_number,VERSION_NUM,VERSION_NUM_LEN);
	version_number[VERSION_NUM_LEN] = '\0';
	memcpy(area,ECU_AREA,ECU_AREA_LEN);
	area[ECU_AREA_LEN] = '\0';
	
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

	/* 鎷兼帴ECU淇℃伅 */
	msgcat_s(sendbuffer, 12, ecuid);
	strcat(sendbuffer, version_msg);
	msgcat_d(sendbuffer, 3, num);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}


int inverter_msg(char *sendbuffer, char* id)
{

	strcat(sendbuffer, id); //inverter UID
	strcat(sendbuffer, "05"); 	 //inverter Type
	strcat(sendbuffer, "00000"); //what guess
	strcat(sendbuffer, "END"); 	 //end

	return 0;
}

/* add inverter ID */
int add_id(const char *msg, int num)
{
	int i, count = 0;
	char inverter_id[13] = {'\0'};
	int fd;
	char buff[50];
	fd = open("/home/data/id", O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{	
		for(i=0; i<num; i++)
		{
			strncpy(inverter_id, &msg[i*15], 12);
			inverter_id[12] = '\0';
			sprintf(buff,"%s,,,,,,\n",inverter_id);
			write(fd,buff,strlen(buff));
			count++;
		}
		
		close(fd);
	}
	echo("/yuneng/limiteid.con","1");
	return count;
}

/* delete inverter ID */
int delete_id(const char *msg, int num)
{
	int i, count = 0;
	char inverter_id[13] = {'\0'};

	for(i=0; i<num; i++)
	{
		strncpy(inverter_id, &msg[i*15], 12);
		inverter_id[12] = '\0';
		delete_line("/home/data/id","/home/data/idtmp",inverter_id,12);
		count++;
	}
	return count;
}

/* clear inverter ID */
int clear_id()
{
	unlink("/home/data/id");
	return 0;

}

/* [A102] ECU Response all inverter id */
int response_inverter_id(const char *recvbuffer, char *sendbuffer)
{
	int i;
	char UID[13];
	/* Head Info*/
	strcpy(sendbuffer, "APS13AAAAAA102AAA0"); 

	{
		/* ECU Message */
		ecu_msg(sendbuffer, ecu.validNum, recvbuffer);

		for(i = 0; i < ecu.validNum;i++)
		{
			sprintf(UID,"%s",inverterInfo[i].uid);
			UID[12] = '\0';
			/* Inverter Message */
			inverter_msg(sendbuffer,UID);		
		}
		
	}
	return 0;
}

/* [A102] EMA Set inverter id */
int set_inverter_id(const char *recvbuffer, char *sendbuffer)
{
	int flag, num;
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	
	//获取设置类型标志:0清除优化器 1添加优化器 2删除优化器
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
		threadRestartTimer(10,TYPE_COMM);

	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A103", timestamp, ack_flag);
	return 102; //返回下一条命令的命令号
}
