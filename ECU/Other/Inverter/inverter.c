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
#include "file.h"
#include "RFM300H.h"
#include "timer.h"
#include "string.h"
#include "led.h"
#include "serverfile.h"

extern unsigned int Heart_times;
extern unsigned int TimeOut_times;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int init_ecu(void)
{

	Read_ECUID(ecu.ECUID6);						//读取ECU ID
	transformECUID(ecu.ECUID6,ecu.ECUID12);		//转换ECU ID
	
	Read_CHANNEL(ecu.Signal_Channel);
	ecu.Channel_char = (ecu.Signal_Channel[0] -'0')*10 +(ecu.Signal_Channel[1] -'0');
	setChannel(ecu.Channel_char);

	Read_IO_INIT_STATU(&ecu.IO_Init_Status);
	ecu.life_energy = get_lifetime_power();
	printf("ECU ID :%s        Signal_Channel:   %s    %d   IO_Init_Status:%x\n",ecu.ECUID12,ecu.Signal_Channel,ecu.Channel_char,ecu.IO_Init_Status);
	return 0;
}

int init_inverter(inverter_info *inverter)
{
	int i;
	char bindstatus = 0;
	unsigned char UID_NUM[2] = {'\0'};
	inverter_info *curinverter = inverter;
	UID_NUM[0] = 0;
	UID_NUM[1] = 0;
	
	Heart_times = 0;
	TimeOut_times = 0;
	
	//???ùóDμ???±??÷????
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		memset(curinverter->uid, 0xff, sizeof(curinverter->uid));			
		curinverter->heart_rate = 0;
		curinverter->off_times = 0;
		curinverter->status.bind_status = 0;
		curinverter->status.mos_status = 0;
		curinverter->status.function_status = 0;
		curinverter->status.heart_Failed_times = 0;
		curinverter->status.pv1_low_voltage_pritection = 0;
		curinverter->status.pv2_low_voltage_pritection = 0;
		curinverter->status.device_Type = 0;
		curinverter->status.channel_failed = 0;
		curinverter->channel = 0;
		curinverter->find_channel = 0;
		curinverter->restartNum = 0;
		curinverter->PV1 = 0;
		curinverter->PV2 = 0;
		curinverter->PI = 0;
		curinverter->PV_Output = 0;
		curinverter->Power1 = 0;
		curinverter->Power2 = 0;
		curinverter->RSSI = 0;
		curinverter->PV1_Energy = 0;
		curinverter->PV2_Energy = 0;
		curinverter->Mos_CloseNum = 0;
		//memset(curinverter->CurCommTime,0x00,15);
		curinverter->Last_PV1_Energy = 0;
		curinverter->Last_PV2_Energy = 0;
		memset(curinverter->LastCommTime,'0',15);
		curinverter->LastCommTime[14] = '\0';
		memset(curinverter->CurCollectTime,'0',15);
		memset(curinverter->LastCollectTime,'0',15);
		curinverter->AveragePower1 = 0;
		curinverter->AveragePower1 = 0;
		
		
	}
	
	//′óEEPROM?D?áè???±??÷D??￠

	Read_UID_NUM((char *)&UID_NUM);
	ecu.validNum = UID_NUM[0] *256 + UID_NUM[1];
	if(ecu.validNum > MAXINVERTERCOUNT)
	{
		ecu.validNum = 0;
	}
	//è?1?μ±?°ó??ˉ?÷êyá??a0￡????′ìáê?μ?3￡áá
	//if(ecu.validNum == 0)
		//LED_on();
	printf("validNum :%d     \n",ecu.validNum);
	curinverter = inverter;
	for(i=0; (i<MAXINVERTERCOUNT && i<ecu.validNum); i++, curinverter++)
	{
		Read_UID((char *)curinverter->uid,(i+1));
		Read_UID_Bind(&bindstatus,(i+1));
		if(bindstatus != 1)			//1 表示绑定成功
		{
			curinverter->status.bind_status = 0; //0表示绑定失败
		}else
		{
			curinverter->status.bind_status = 1; //0表示绑定失败
		}

		Read_UID_Channel((char *)&curinverter->channel,(i+1));
		
		printf("uid%d: %02x%02x%02x%02x%02x%02x   bind:%d channel:%d\n",(i+1),curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5],curinverter->status.bind_status,curinverter->channel);
	}
	return 0;
}

int add_inverter(inverter_info *inverter,int num,char *uidstring)
{
	int i;
	
	unsigned char UID_NUM[2] = {'\0'};

	inverter_info *curinverter = inverter;
	UID_NUM[0] = num/256;
	UID_NUM[1] = num%256;
	//SEGGER_RTT_printf(0, "111111111111111 %x %x\n",UID_NUM[0],UID_NUM[1]);	
	
	//?òEEPROMD′è?êy?Y
	Write_UID_NUM((char *)&UID_NUM);

	curinverter = inverter;
	for(i=0; (i<MAXINVERTERCOUNT && i<num); i++, curinverter++)
	{

		Write_UID(&uidstring[0+(i*6)],(i+1));
		Write_UID_Bind(0x00,(i+1));
		Write_UID_Channel(0x01,(i+1));
		//Write_UID_Channel(0x10,(i+1));
		//SEGGER_RTT_printf(0, "add_inverter uid%d: %02x%02x%02x%02x%02x%02x  \n",(i+1),uidstring[0+(i*6)],uidstring[1+(i*6)],uidstring[2+(i*6)],uidstring[3+(i*6)],uidstring[4+(i*6)],uidstring[5+(i*6)]);		
	}
	return 0;
}



