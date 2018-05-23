/*****************************************************************************/
/* File      : inverter.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "inverter.h"
#include "SEGGER_RTT.h"
#include "timer.h"
#include "string.h"
#include "led.h"
#include "serverfile.h"
#include "stdio.h"
#include "zigbee.h"
#include "debug.h"
#include "bind_inverters.h"
#include "rsdFunction.h"
#include "stdlib.h"
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int init_ecu(void)
{
	Read_ECUID(ecu.ECUID12);						//读取ECU ID
	transformECUID(ecu.ECUID6,ecu.ECUID12);		//转换ECU ID
	//获取panid
	ecu.panid = get_panid();
	ecu.channel = get_channel();
	ecu.today_energy = get_today_energy();
	ecu.count = 0;
	ecu.validNum = 0;
	ecu.curSequence = 0;
	Read_IO_INIT_STATU(&ecu.IO_Init_Status);
	ecu.flag_ten_clock_getshortaddr = 1;			//ZK
	ecu.life_energy = get_lifetime_power();
	zb_change_ecu_panid();
	memset(ecu.curTime,'0',14);
	memset(ecu.JsonTime,'0',14);
	ecu.curTime[14] = '\0';
	ecu.JsonTime[14] = '\0';
	ecu.polling_total_times = 0;
	ecu.idUpdateFlag = 0;
	ecu.ThirdIDUpdateFlag = 0;
	ecu.thirdCommNum = 0;
	return 0;
}

int init_inverter(inverter_info *inverter)
{
	int i;
	char flag_limitedid = '0';				//限定ID标志
	FILE *fp;
	inverter_info *curinverter = inverter;
	unsigned char RSDStatus = atoi(&ecu.IO_Init_Status);
	printf("RSDStatus:%d\n",RSDStatus);
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		memset(curinverter->uid, 0xff, sizeof(curinverter->uid));	
		curinverter->shortaddr = 0;
		curinverter->model = 0xff;
		curinverter->zigbee_version = 0;
		curinverter->version = 0;
		
		curinverter->heart_rate = 0;
		curinverter->off_times = 0;
		curinverter->status.comm_failed3_status = 0;
		curinverter->status.function_status = 1;
		curinverter->status.pv1_low_voltage_pritection = 0;
		curinverter->status.pv2_low_voltage_pritection = 0;
		curinverter->status.device_Type = 0;
		curinverter->status.comm_status = 0;
		curinverter->status.bindflag = 0;
		curinverter->status.flag = 0;
		curinverter->status.mos_status = 1;
		curinverter->status.last_mos_status= 1;
		curinverter->status.last_function_status = 1;
		curinverter->status.last_pv1_low_voltage_pritection = 0;
		curinverter->status.last_pv2_low_voltage_pritection = 0;
		curinverter->status.turn_on_collect_data = 0;
		curinverter->temperature = 100;
		memset(&curinverter->parameter_status,0x00,2);
		memset(&curinverter->config_status,0x00,1);
		
		curinverter->restartNum = 0;
		curinverter->PV1 = 0;
		curinverter->PV2 = 0;
		curinverter->PI = 0;
		curinverter->PI2 = 0;
		curinverter->PV_Output = 0;
		curinverter->PI_Output = 0;
		curinverter->Power1 = 0;
		curinverter->Power2 = 0;
		curinverter->Power_Output = 0;
		curinverter->RSSI = 0;
		curinverter->PV1_Energy = 0;
		curinverter->PV2_Energy = 0;
		curinverter->PV_Output_Energy = 0;
		curinverter->Mos_CloseNum = 0;
		
		curinverter->Last_PV1_Energy = 0;
		curinverter->Last_PV2_Energy = 0;
		curinverter->Last_PV_Output_Energy = 0;
		memset(curinverter->LastCommTime,'0',15);
		curinverter->LastCommTime[14] = '\0';
		memset(curinverter->LastCollectTime,'0',15);
		curinverter->LastCollectTime[14] = '\0';
		curinverter->AveragePower1 = 0;
		curinverter->AveragePower2 = 0;
		curinverter->AveragePower_Output = 0;
		curinverter->EnergyPV1 = 0;
		curinverter->EnergyPV2 = 0;
		curinverter->EnergyPV_Output = 0;
		curinverter->no_getdata_num = 0;
		//初始化配置状态为RSD系统状态
		curinverter->config_status.rsd_config_status = RSDStatus;
	}
	
	while(1) {
		ecu.validNum = get_id_from_file(inverter);
		if (ecu.validNum > 0) {
			break; //直到逆变器数量大于0时退出循环
		} else {
			printmsg(ECU_DBG_COMM,"please Input Inverter ID---------->"); //提示用户输入逆变器ID
			rt_thread_delay(20*RT_TICK_PER_SECOND);
		}
	}

	fp = fopen("/config/limiteid.con", "r");
	if(fp)
	{
		flag_limitedid = fgetc(fp);
		fclose(fp);
	}
	
	if ('1' == flag_limitedid) {
		bind_inverters(); //绑定逆变器
		fp = fopen("/config/limiteid.con", "w");
		if (fp) {
			fputs("0", fp);
			fclose(fp);
		}
		//process_rsd_enable_boardcast();//广播使能RSD
	}
	//判断是否需要广播参数设置指令  	先广播，然后再每台单播
	process_rsdFunction_all();
	//判断是否需要单播参数设置指令		进行每台单播
	process_rsdFunction();	
	
	
	return 0;
}


int init_inverter_A103(inverter_info *inverter)
{
	int i;
	inverter_info *curinverter = inverter;
	
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		memset(curinverter->uid, 0xff, sizeof(curinverter->uid));	
		curinverter->shortaddr = 0;
		curinverter->model = 0xff;
		curinverter->zigbee_version = 0;
		curinverter->version = 0;
		
		curinverter->heart_rate = 0;
		curinverter->off_times = 0;
		curinverter->status.comm_failed3_status = 0;
		curinverter->status.function_status = 1;
		curinverter->status.pv1_low_voltage_pritection = 0;
		curinverter->status.pv2_low_voltage_pritection = 0;
		curinverter->status.device_Type = 0;
		curinverter->status.comm_status = 0;
		curinverter->status.bindflag = 0;
		curinverter->status.flag = 0;
		curinverter->status.mos_status = 1;
		curinverter->status.last_mos_status= 1;
		curinverter->status.last_function_status = 1;
		curinverter->status.last_pv1_low_voltage_pritection = 0;
		curinverter->status.last_pv2_low_voltage_pritection = 0;
		
		curinverter->restartNum = 0;
		curinverter->PV1 = 0;
		curinverter->PV2 = 0;
		curinverter->PI = 0;
		curinverter->PI2 = 0;
		curinverter->PV_Output = 0;
		curinverter->PI_Output = 0;
		curinverter->Power1 = 0;
		curinverter->Power2 = 0;
		curinverter->Power_Output = 0;
		curinverter->RSSI = 0;
		curinverter->PV1_Energy = 0;
		curinverter->PV2_Energy = 0;
		curinverter->PV_Output_Energy = 0;
		curinverter->Mos_CloseNum = 0;
		
		curinverter->Last_PV1_Energy = 0;
		curinverter->Last_PV2_Energy = 0;
		curinverter->Last_PV_Output_Energy = 0;
		memset(curinverter->LastCommTime,'0',15);
		curinverter->LastCommTime[14] = '\0';
		memset(curinverter->LastCollectTime,'0',15);
		curinverter->LastCollectTime[14] = '\0';
		curinverter->AveragePower1 = 0;
		curinverter->AveragePower2 = 0;
		curinverter->AveragePower_Output = 0;
		curinverter->EnergyPV1 = 0;
		curinverter->EnergyPV2 = 0;
		curinverter->EnergyPV_Output = 0;
		
	}

	ecu.validNum = get_id_from_file(inverter);

	return 0;
}

