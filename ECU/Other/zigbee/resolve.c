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
#include "variation.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int resolvedata_OPT700_RS(char *inverter_data, struct inverter_info_t *inverter)
{
	char status = 0;
	unsigned char pre_model = inverter->model;  //�ֽ�ԭ����model����   ����ͻ�ȡ���Ĳ�ͬ ����model��
	unsigned short pre_version = inverter->version;

	//������һ�ֱ���״̬����
	inverter->status.last_mos_status = inverter->status.mos_status;
	inverter->status.last_function_status = inverter->status.function_status;
	inverter->status.last_pv1_low_voltage_pritection = inverter->status.pv1_low_voltage_pritection;
	inverter->status.last_pv2_low_voltage_pritection = inverter->status.pv2_low_voltage_pritection;
	inverter->last_RSDTimeout = inverter->RSDTimeout;

	inverter->PV_Output = inverter_data[4]*256 + inverter_data[5];
	inverter->PV1 = inverter_data[6]*256 + inverter_data[7];
	inverter->PV2 = inverter_data[8]*256 + inverter_data[9];

	inverter->Power_Output = inverter_data[10]*256 + inverter_data[11];
	inverter->Power1 = inverter_data[12]*256 + inverter_data[13];
	inverter->Power2 = inverter_data[14]*256 + inverter_data[15];
	
	inverter->PI_Output = inverter_data[16]*256 + inverter_data[17];
	inverter->PI = inverter_data[18]*256 + inverter_data[19];
	inverter->PI2 = inverter_data[20]*256 + inverter_data[21];

	inverter->temperature =  inverter_data[23];
	
	inverter->PV_Output_Energy = inverter_data[24]*65536*256 + inverter_data[25]*65536 + inverter_data[26]*256 + inverter_data[27];
	inverter->PV1_Energy = inverter_data[28]*65536*256 + inverter_data[29]*65536 + inverter_data[30]*256 + inverter_data[31];
	inverter->PV2_Energy = inverter_data[32]*65536*256 + inverter_data[33]*65536 + inverter_data[34]*256 + inverter_data[35];

	inverter->version = inverter_data[36]*256 + inverter_data[37];

	inverter->model =  inverter_data[38];	//��ȡ�����룬�����������00��ʾ��ʷ����
									//����IDֵ����ʷ���͸���Ϊ��Ӧ�Ļ���
	if(inverter->model == 0x00 ||inverter->model == 0xff)
	{
		if(!memcmp(inverter->uid,"601",3))
		{
			inverter->model  = 0x01;
		}else if (!memcmp(inverter->uid,"611",3))	//RSD������λ����Ϊ0xff
		{
			inverter->model  = 0x02;
		}
	}
	
	if((inverter_data[38] == 0xD0) || (inverter_data[38] == 0xFF))	//����豸
	{
		inverter->status.device_Type = 1;		//����豸
	}else if((inverter_data[38] == 0xD1))
	{
		inverter->status.device_Type = 0;		//�����豸
	}else
	{
		inverter->status.device_Type = 1;		//���������ֵ������豸
	}

	//�ɼ�����״̬
	status = (inverter_data[39] & 1);
	inverter->status.function_status = status;

	//�ɼ�PV1Ƿѹ����״̬
	status = (inverter_data[39] & ( 1 << 1 ) ) >> 1;
	inverter->status.pv1_low_voltage_pritection= status;

	//�ɼ�PV2Ƿѹ����״̬
	status = (inverter_data[39] & ( 1 << 2 )) >> 2;
	inverter->status.pv2_low_voltage_pritection= status;

	inverter->heart_rate = inverter_data[42]*256 + inverter_data[43];
	inverter->off_times = inverter_data[44]*256 + inverter_data[45];
	inverter->Mos_CloseNum = inverter_data[46];
	//������ʱʱ��
	inverter->RSDTimeout = inverter_data[50];
	//����PV1 PV2��ʱ����
	inverter->PV1_low_voltageNUM = inverter_data[51]*256+inverter_data[52];
	inverter->PV2_low_voltageNUM = inverter_data[53]*256+inverter_data[54];
	if(inverter->PV_Output > 0) 
		inverter->status.mos_status = 1;
	else
		inverter->status.mos_status = 0;

	memcpy(&inverter->parameter_status,&inverter_data[56],2);
	//ͨѶ��BUG�������
	//�жϺ���һ��PV1��PV2��������Ƿ���ͬ������PV1 PV2������ȴ���0
	if((inverter->PV1_Energy == inverter->Last_PV1_Energy)&&(inverter->PV2_Energy == inverter->Last_PV2_Energy) && (inverter->PV1_Energy>0) && (inverter->PV2_Energy>0))
		return -1;
	apstime(inverter->LastCommTime);
	inverter->LastCommTime[14] = '\0';

	if((pre_model != inverter->model) ||(pre_version != inverter->version))	//���model�仯��  ��ECU��ID���±�־λ��1
	{
		ecu.idUpdateFlag = 1;
	}
	return 0;
}
