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
#include "serverfile.h"
#include "debug.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int process_rsd_enable_boardcast(void)
{
	int i =0;
	//先广播三次
	for(i = 0;i<3;i++)
	{
		zb_set_heartSwitch_boardcast(1,2,0);
		rt_hw_s_delay(1);
	}
	return 0;
}

int process_rsd_single(void)
{
	//如果系统RSD状态和某台RSD设备不同，更改
	for(ecu.curSequence = 0;ecu.curSequence<ecu.validNum;ecu.curSequence++)
	{
		if((inverterInfo[ecu.curSequence].status.comm_failed3_status == 1) && (inverterInfo[ecu.curSequence].shortaddr !=0)  && (inverterInfo[ecu.curSequence].status.function_status != atoi(&ecu.IO_Init_Status)))
		{	
			if(ecu.IO_Init_Status == '0')
			{
				zb_set_heartSwitch_single(&inverterInfo[ecu.curSequence],2,2,0);	
			}else
			{
				zb_set_heartSwitch_single(&inverterInfo[ecu.curSequence],1,2,0);
			}
					
		}
	}
	return 0;
}

int process_rsdFunction_all(void)
{
	unsigned char rsdChangeFunctionStatus = 1;
	unsigned char rsdOnOffStatus = 2;
	unsigned char rsdTimeout = 0;
	//判断是否需要改变功能状态
	if (rsdFunction_need_change()) {
		getChangeFunctionStatus(&rsdChangeFunctionStatus,&rsdOnOffStatus,&rsdTimeout);
		//修改功能状态
		changeRSDFunctionOfInverters(rsdChangeFunctionStatus,rsdOnOffStatus,rsdTimeout);
		
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
int saveChangeFunctionStatus(unsigned char FunctionStatus,unsigned char onoffstatus,unsigned char rsdTimeout)
{
	FILE *fp;
	char buffer[20] = {'\0'};
	
	fp = fopen("/tmp/funStatu.con", "w");
	if (fp) {
		sprintf(buffer,"%d,%d,%03d",FunctionStatus,onoffstatus,rsdTimeout);
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
int getChangeFunctionStatus(unsigned char *FunctionStatus,unsigned char *onoffstatus,unsigned char *rsdTimeout)
{
	FILE *fp;
	char buffer[20] = {'\0'};
	char strfunstatus[2] = {'\0'};
	char stronoff[2] = {'\0'};
	char strrsdTimeout[4] = {'\0'};
	fp = fopen("/tmp/funStatu.con", "r");
	if (fp) {
		fgets(buffer, 20, fp);
		fclose(fp);
		memcpy(strfunstatus,&buffer[0],1);
		memcpy(stronoff,&buffer[2],1);
		memcpy(strrsdTimeout,&buffer[4],3);
		*FunctionStatus = atoi(strfunstatus);
		*onoffstatus = atoi(stronoff);
		*rsdTimeout = atoi(strrsdTimeout);
		return 0;
	}
	return 0; 
}

void changeRSDFunctionOfInverters(unsigned char changeFunctionStatus,unsigned char onoffStatus,unsigned char rsdTimeout)
{
	int i = 0,j = 0,ret = 0;
	inverter_info *curinverter = inverterInfo;
	
	//先广播三次
	for(i = 0;i<3;i++)
	{
		zb_set_heartSwitch_boardcast(changeFunctionStatus,onoffStatus,rsdTimeout);
		rt_hw_s_delay(1);
	}
	
	
	//单播每一台
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//有效逆变器轮训
	{
		for(j = 0;j < 3;j++)
		{
			ret = zb_set_heartSwitch_single(curinverter,changeFunctionStatus,onoffStatus,rsdTimeout);
			if(ret == 1) break;
			
		}
		
	}
	
}

int get_rsd_function_flag(char *id,unsigned char *FunctionStatus,unsigned char *onoffstatus,unsigned char *rsdTimeout)
{
	int flag1=0;
	FILE *fp;
	char data[200];
	char splitdata[5][32];
	char strfunstatus[2] = {'\0'};
	char stronoff[2] = {'\0'};
	char strrsdTimeout[4] = {'\0'};
	fp = fopen("/tmp/setrsd", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,5*32);
			splitString(data,splitdata);
			strcpy(id, splitdata[0]);
			memcpy(strfunstatus,&splitdata[1],1);
			memcpy(stronoff,&splitdata[2],1);
			memcpy(strrsdTimeout,&splitdata[3],3);
			*FunctionStatus = atoi(strfunstatus);
			*onoffstatus = atoi(stronoff);
			*rsdTimeout = atoi(strrsdTimeout);
			flag1 = 1;
			break;
		}
		fclose(fp);
	}
	return flag1;
}

int clear_rsd_function_flag(char *id)
{
	delete_line("/tmp/setrsd","/tmp/setrsd.t",id,12);
	return 0;
}


void insertSetRSDInfo(unsigned short num,char *msg)
{
	int i;
	char inverter_id[13] = {'\0'};
	int fd;
	char buff[50];
	char strfunstatus[2] = {'\0'};
	char stronoff[2] = {'\0'};
	char strrsdTimeout[4] = {'\0'};
	unsigned char FunctionStatus,onoffstatus,rsdTimeout;
	fd = open("/tmp/setrsd", O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{	
		for(i=0; i<num; i++)
		{
			strncpy(inverter_id, &msg[i*20], 12);
			inverter_id[12] = '\0';
			memcpy(strfunstatus,&msg[i*20+12],1);
			memcpy(stronoff,&msg[i*20+13],1);
			memcpy(strrsdTimeout,&msg[i*20+14],3);
			FunctionStatus = atoi(strfunstatus);
			onoffstatus = atoi(stronoff);
			rsdTimeout = atoi(strrsdTimeout);
			sprintf(buff,"%s,%d,%d,%03d\n",inverter_id,FunctionStatus,onoffstatus,rsdTimeout);
			write(fd,buff,strlen(buff));
		}
			
		close(fd);
	}

}

int process_rsdFunction(void)
{
	char id[13] = {'\0'};
	unsigned char FunctionStatus = 0;
	unsigned char onoffstatus = 0;
	unsigned char rsdTimeout = 0;
	inverter_info *curinverter = inverterInfo;
	int i,k,ret = 0;
	while(1)
	{
		curinverter = inverterInfo;
		memset(id, '\0', 13);
		if(!get_rsd_function_flag(id,&FunctionStatus,&onoffstatus,&rsdTimeout))
			break;
		clear_rsd_function_flag(id);
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++)
		{
			if(!strcmp(id, curinverter->uid))
			{
				for(k = 0;k < 3;k++)
				{
					ret = zb_set_heartSwitch_single(curinverter,FunctionStatus,onoffstatus,rsdTimeout);
					if(ret == 1) break;
						
				}
				print2msg(ECU_DBG_COLLECT,curinverter->uid, "zb_set_heartSwitch_single");

				break;
			}
			curinverter++;
		}
	}

	return 0;


}

