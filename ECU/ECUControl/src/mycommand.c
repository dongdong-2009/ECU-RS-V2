/*****************************************************************************/
/*  File      : mycommand.c                                                  */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-13 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "rthw.h"
#include "threadlist.h"
#include <rtthread.h>
#include "serverfile.h"
#include "thftpapi.h"
#include <dfs_posix.h>

//定时器
rt_timer_t readtimer;

//定时器超时函数
static void reboottimeout(void* parameter)
{
	reboot();
}

void reboot_timer(int timeout)			//zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{
	readtimer = rt_timer_create("read", /* 定时器名字为 read */
					reboottimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
}

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int mysystem(const char *command)
{
	int res;

	print2msg(ECU_DBG_CONTROL_CLIENT,"Execute:",(char*) command);
	//需要的命令在此添加
	if(!memcmp(command,"reboot",6))
	{
		res = 0;
		//通过定时器复位程序
		reboot_timer(10);
	}else if(!memcmp(command,"restart UPDATE",14))
	{
		restartThread(TYPE_UPDATE);
		res = 0;
	}else if(!memcmp(command,"ftpput",6))	//上传数据
	{
		char sourcePath[50],destPath[50];
		//分割字符串为  命令 [本地源路径] [远程目标路径]
		splitSpace((char *)command,sourcePath,destPath);
		printf("cmd:%s\n",command);
		printf("%s,%s,%s\n","ftpput",sourcePath,destPath);
		//上传数据
		res = putfile(destPath,sourcePath);

	}else if(!memcmp(command,"ftpget",6))	//下载数据
	{
		char sourcePath[50],destPath[50];
		//分割字符串为  命令 [本地源路径] [远程目标路径]
		printf("cmd:%s\n",command);
		splitSpace((char *)command,sourcePath,destPath);
		printf("%s,%s,%s\n","ftpget",sourcePath,destPath);
		//下载数据
		res = getfile(destPath,sourcePath);
	}else if(!memcmp(command,"rm",2))
	{
		char path[50];
		memcpy(path,&command[3],(strlen(command)-3));
		unlink(path);
	}
		
	printdecmsg(ECU_DBG_CONTROL_CLIENT,"res",res);
	if(-1 == res){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: system error.");
		return CMD_ERROR;
	}
	else{

		if(0 == res){
			printmsg(ECU_DBG_CONTROL_CLIENT,"Execute successfully.");
		}
		else{
			printdecmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: shell failed", res);
			return CMD_ERROR;
		}

	}
	return SUCCESS;
}
