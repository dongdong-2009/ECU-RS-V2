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
	//�ж��Ƿ���Ҫ�ı书��״̬
	if (rsdFunction_need_change()) {
		rsdCurFunctionStatus = atoi(&ecu.IO_Init_Status);
		rsdChangeFunctionStatus = getChangeFunctionStatus();
		//�޸Ĺ���״̬
		changeRSDFunctionOfInverters(rsdCurFunctionStatus, rsdChangeFunctionStatus);
		
		//���湦��״̬
		if(rsdChangeFunctionStatus == 1)
			Write_IO_INIT_STATU("1");
		else
			Write_IO_INIT_STATU("0");
		
		//��ձ�־λ
		unlink("/tmp/rsdFun.con");
		unlink("/tmp/funStatu.con");
	}
	return 0;
	
}

/* �ж��Ƿ���Ҫ�ı�RSD���ع��� */
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

// ������Ҫ�ı��RSD���ܱ�־λ
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

//�����޸ı�־λ
int save_rsdFunction_change_flag(void)
{
	echo("/tmp/rsdFun.con","1");
	return 0;
}

// ��ȡ��Ҫ�ı��RSD���ܱ�־λ
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
	return 0; //δ֪�ŵ�
}

void changeRSDFunctionOfInverters(unsigned char curFunctionStatus, unsigned char changeFunctionStatus)
{
	int i = 0,j = 0,ret = 0;
	inverter_info *curinverter = inverterInfo;
	
	//�ȹ㲥����
	for(i = 0;i<3;i++)
	{
		zb_set_heartSwitch_boardcast(changeFunctionStatus);
		rt_hw_s_delay(1);
	}
	
	
	//����ÿһ̨
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//��Ч�������ѵ
	{
		for(j = 0;j < 3;j++)
		{
			ret = zb_set_heartSwitch_single(curinverter,changeFunctionStatus);
			if(ret == 1) break;
			
		}
		
	}
	
}
