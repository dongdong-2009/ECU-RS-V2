/*****************************************************************************/
/*  File      : resolve.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-10-13 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "resolve.h"
#include "debug.h"
#include <string.h>
#include "serverfile.h"
#include "rtc.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int resolvedata_OPT700_RS(char *inverter_data, struct inverter_info_t *inverter)
{
	char status = 0;
	unsigned short last_PV_output;		//最后一次开关机状态
	unsigned char last_function_status;	//最后一次功能状态
	unsigned char last_pv1_low_voltage_pritection;	//最后一次PV1欠压状态
	unsigned char last_pv2_low_voltage_pritection;	//最后一次PV2欠压状态
	
	//保存上一轮报警状态数据
	last_PV_output = inverter->PV_Output;
	last_function_status = inverter->status.function_status;
	last_pv1_low_voltage_pritection = inverter->status.pv1_low_voltage_pritection;
	last_pv2_low_voltage_pritection = inverter->status.pv2_low_voltage_pritection;

	inverter->status.comm_failed3_status = 1;	//设置为开机状态
	apstime(inverter->LastCommTime);
	inverter->LastCommTime[14] = '\0';

	
	inverter->PV_Output = inverter_data[4]*256 + inverter_data[5];
	inverter->PV1 = inverter_data[6]*256 + inverter_data[7];
	inverter->PV2 = inverter_data[8]*256 + inverter_data[9];

	inverter->Power_Output = inverter_data[10]*256 + inverter_data[11];
	inverter->Power1 = inverter_data[12]*256 + inverter_data[13];
	inverter->Power2 = inverter_data[14]*256 + inverter_data[15];
	
	inverter->PI_Output = inverter_data[16]*256 + inverter_data[17];
	inverter->PI = inverter_data[18]*256 + inverter_data[19];
	inverter->PI2 = inverter_data[20]*256 + inverter_data[21];
	
	inverter->PV_Output_Energy = inverter_data[24]*65536*256 + inverter_data[25]*65536 + inverter_data[26]*256 + inverter_data[27];
	inverter->PV1_Energy = inverter_data[28]*65536*256 + inverter_data[29]*65536 + inverter_data[30]*256 + inverter_data[31];
	inverter->PV2_Energy = inverter_data[32]*65536*256 + inverter_data[33]*65536 + inverter_data[34]*256 + inverter_data[35];

	inverter->version = inverter_data[36]*256 + inverter_data[37];
	
	if((inverter_data[38] == 0xD0))	//监控设备
	{
		inverter->status.device_Type = 1;		//监控设备
	}else if((inverter_data[38] == 0xD1))
	{
		inverter->status.device_Type = 0;		//开关设备
	}else
	{
		;											//如果是其他值，保持原来的不变
	}

	//采集功能状态
	status = (inverter_data[39] & 1);
	inverter->status.function_status = status;

	//采集PV1欠压保护状态
	status = (inverter_data[39] & ( 1 << 1 ) ) >> 1;
	inverter->status.pv1_low_voltage_pritection= status;

	//采集PV2欠压保护状态
	status = (inverter_data[39] & ( 1 << 2 )) >> 2;
	inverter->status.pv2_low_voltage_pritection= status;

	inverter->heart_rate = inverter_data[42]*256 + inverter_data[43];
	inverter->off_times = inverter_data[44]*256 + inverter_data[45];
	inverter->Mos_CloseNum = inverter_data[46];


	create_alarm_record(last_PV_output,last_function_status,last_pv1_low_voltage_pritection,last_pv2_low_voltage_pritection,inverter);
	return 0;
}
