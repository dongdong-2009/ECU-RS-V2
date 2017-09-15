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
		reboot();
	}else if(!memcmp(command,"restart UPDATE",14))
	{
		restartThread(TYPE_UPDATE);
		res = 0;
	}else if(!memcmp(command,"ftpput",6))
	{
		//上传数据
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
