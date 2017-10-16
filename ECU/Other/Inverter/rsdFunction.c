/*****************************************************************************/
/*  File      : rsdFunction.c                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-10-16 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "rsdFunction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include <dfs_posix.h> 
#include "channel.h"
#include "zigbee.h"
#include "serverfile.h"
#include "rthw.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int process_rsdFunction(void)
{
	unsigned char rsdChangeFunctionStatus = 1;
	unsigned char rsdCurFunctionStatus = 1;
	//判断是否需要改变功能状态
	if (rsdFunction_need_change()) {
		rsdCurFunctionStatus = atoi(&ecu.IO_Init_Status);
		rsdChangeFunctionStatus = getChangeFunctionStatus();
		//修改功能状态
		changeRSDFunctionOfInverters(rsdCurFunctionStatus, rsdChangeFunctionStatus);
		
		//保存功能状态
		if(rsdChangeFunctionStatus == 1)
			Write_IO_INIT_STATU("1");
		else
			Write_IO_INIT_STATU("0");
		
		//清空标志位
		unlink("/tmp/rsdFun.con");
		unlink("/tmp/funStatu.con");
	}
	return 0;
	
}

/* 判断是否需要改变RSD开关功能 */
int rsdFunction_need_change()
{
	FILE *fp;
	char buff[2] = {'\0'};

	fp = fopen("/tmp/rsdFun.con", "r");
	if (fp) {
		fgets(buff, 2, fp);
		fclose(fp);
	}

	return ('1' == buff[0]);
}

// 保存需要改变的RSD功能标志位
int saveChangeFunctionStatus(unsigned char FunctionStatus)
{
	FILE *fp;
	char buffer[3] = {'\0'};
	
	fp = fopen("/tmp/funStatu.con", "w");
	if (fp) {
		sprintf(buffer,"%c",FunctionStatus);
		fputs(buffer, fp);
		fclose(fp);
	}
	return 0;
}

//更改修改标志位
int save_rsdFunction_change_flag(void)
{
	echo("/tmp/rsdFun.con","1");
	return 0;
}

// 获取需要改变的RSD功能标志位
int getChangeFunctionStatus(void)
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/funStatu.con", "r");
	if (fp) {
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 0; //未知信道
}

void changeRSDFunctionOfInverters(unsigned char curFunctionStatus, unsigned char changeFunctionStatus)
{
	int i = 0,j = 0,ret = 0;
	inverter_info *curinverter = inverterInfo;
	
	//先广播三次
	for(i = 0;i<3;i++)
	{
		zb_set_heartSwitch_boardcast(changeFunctionStatus);
		rt_hw_s_delay(1);
	}
	
	
	//单播每一台
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//有效逆变器轮训
	{
		for(j = 0;j < 3;j++)
		{
			ret = zb_set_heartSwitch_single(curinverter,changeFunctionStatus);
			if(ret == 1) break;
			
		}
		
	}
	
}
