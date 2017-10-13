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
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//A160设置RSD功能开关
int set_rsd_function_switch(const char *recvbuffer, char *sendbuffer)
{
	int flag;
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	
	//获取设置类型标志位: 0清除逆变器; 1添加逆变器; 2删除逆变器
	sscanf(&recvbuffer[30], "%1d", &flag);

	//获取时间戳
	strncpy(timestamp, &recvbuffer[31], 14);
	timestamp[14] = '\0';
	printf("recvbuffer:%s\n",recvbuffer);
	printf("flag:%d\n",flag);
	printf("timestamp:%s\n",timestamp);
	

	//数据库打开成功，进行数据操作
	switch(flag)
	{
		case 0:
			//RSD功能关闭
			printmsg(ECU_DBG_CONTROL_CLIENT,"Write_IO_INIT_STATU(0)");
			Write_IO_INIT_STATU("0");
			ecu.IO_Init_Status = '0';
			break;
		case 1:
			//RSD功能打开
			printmsg(ECU_DBG_CONTROL_CLIENT,"Write_IO_INIT_STATU(1)");
			Write_IO_INIT_STATU("1");
			ecu.IO_Init_Status = '1';
			break;
		default:
			ack_flag = FORMAT_ERROR; //格式错误
			break;
	}

	//拼接应答消息
	msg_ACK(sendbuffer, "A160", timestamp, ack_flag);
	return 0; //返回下一个执行命令的命令号
}
