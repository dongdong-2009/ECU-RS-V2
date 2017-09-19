/*****************************************************************************/
/* File      : rfm300h.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-04 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "RFM300H.h"
#include "SEGGER_RTT.h"
#include "rthw.h"
#include "string.h"
#include "rtc.h"
#include "serverfile.h"
#include "debug.h"


volatile unsigned int Heart_times = 0;
volatile unsigned int TimeOut_times = 0;

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
//#define CMT2300_DEBUG

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int RFM300_Bind_Uid(char *ECUID,char *UID,char channel,char rate,char *ver)
{
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[64] = {'\0'};
	char Recvdata[64] = {'\0'};

	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;
	Senddata[3]=0x08;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=channel;
	Senddata[17]=rate;
	Senddata[18]=0x00;
	Senddata[19]=0x00;
	Senddata[20]=0x00;
	Senddata[21]=0x00;
	Senddata[22]=0x00;
	Senddata[23]=0x00;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
		
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;
#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n",Senddata[i]);
#endif
	for(i=0;i<1;i++){
		SendMessage((unsigned char *)Senddata,28);
		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD8)&&		//表示绑定成功
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{
			*ver = Recvdata[18];
			SEGGER_RTT_printf(0, "RFM300_Bind_Uid %02x%02x%02x%02x%02x%02x\n",Senddata[10],Senddata[11],Senddata[12],Senddata[13],Senddata[14],Senddata[15]);
			//绑定成功，返回1
			return 1;		
		}
		else
		{
			rt_hw_us_delay(20);
			continue;
		}
			
	}
	//绑定失败，返回0
	return 0;
}

//ECU心跳函数
int RFM300_Heart_Beat(char *ECUID,inverter_info * cur_inverter)
{
	char status = 0;
	int i,check = 0;
	unsigned char RF_leng = 0;
	unsigned char last_mos_status;		//最后一次开关机状态
	unsigned char last_function_status;	//最后一次功能状态
	unsigned char last_pv1_low_voltage_pritection;	//最后一次PV1欠压状态
	unsigned char last_pv2_low_voltage_pritection;	//最后一次PV2欠压状态
	
	char Senddata[64] = {'\0'};
	char Recvdata[64] = {'\0'};
	
	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=0x00;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=cur_inverter->uid[0];
	Senddata[11]=cur_inverter->uid[1];
	Senddata[12]=cur_inverter->uid[2];
	Senddata[13]=cur_inverter->uid[3];
	Senddata[14]=cur_inverter->uid[4];
	Senddata[15]=cur_inverter->uid[5];
	Senddata[16]=(Heart_times/16777216)%256;
	Senddata[17]=(Heart_times/65536)%256;
	Senddata[18]=(Heart_times/256)%256;
	Senddata[19]=(Heart_times)%256;
	Senddata[20]=(TimeOut_times/16777216)%256;
	Senddata[21]=(TimeOut_times/65536)%256;
	Senddata[22]=(TimeOut_times/256)%256;
	Senddata[23]=(TimeOut_times)%256;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;

#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif
	
	for(i=0;i<1;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		//printf("%d\n",RF_leng);
		if((RF_leng==46)&&			
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{
			Heart_times++;
			apstime(cur_inverter->LastCommTime);
			cur_inverter->LastCommTime[14] = '\0';
			if((Recvdata[3] == 0xD0))	//监控设备
			{
				cur_inverter->status.device_Type = 1;		//监控设备
			}else if((Recvdata[3] == 0xD1))
			{
				cur_inverter->status.device_Type = 0;		//开关设备
			}else
			{
				;											//如果是其他的值，保持原来的设备不变
			}
			
			cur_inverter->PV1 = Recvdata[16] * 256 + Recvdata[17];
			cur_inverter->PV2 = Recvdata[18] * 256 + Recvdata[19];
			cur_inverter->PI = Recvdata[20];
			cur_inverter->PV_Output = (Recvdata[21]*256) + Recvdata[22];
			cur_inverter->Power1 = (Recvdata[23]*256) + Recvdata[24];
			cur_inverter->Power2 = (Recvdata[25]*256) + Recvdata[26];
			cur_inverter->off_times = Recvdata[27]*256+Recvdata[28];
			cur_inverter->heart_rate = Recvdata[29]*256+Recvdata[30];

			//保存上一轮报警状态数据
			last_mos_status = cur_inverter->status.comm_failed3_status;
			last_function_status = cur_inverter->status.function_status;
			last_pv1_low_voltage_pritection = cur_inverter->status.pv1_low_voltage_pritection;
			last_pv2_low_voltage_pritection = cur_inverter->status.pv2_low_voltage_pritection;
			
			cur_inverter->status.comm_failed3_status = 1;	//设置为开机状态	
			//采集功能状态
			status = (Recvdata[31] & 1);
			cur_inverter->status.function_status = status;
			//采集PV1欠压保护状态
			status = (Recvdata[31] & ( 1 << 1 ) ) >> 1;
			cur_inverter->status.pv1_low_voltage_pritection= status;
			//采集PV2欠压保护状态
			status = (Recvdata[31] & ( 1 << 2 )) >> 2;
			cur_inverter->status.pv2_low_voltage_pritection= status;
			
			cur_inverter->RSSI = Recvdata[32];
			
			//PV1累计发电量
			cur_inverter->PV1_Energy = (Recvdata[33]*256*65536)+(Recvdata[34]*65536)+(Recvdata[35]*256)+Recvdata[36];
			//PV2累计发电量
			cur_inverter->PV2_Energy = (Recvdata[37]*256*65536)+(Recvdata[38]*65536)+(Recvdata[39]*256)+Recvdata[40];
			//MOS管关断次数
			cur_inverter->Mos_CloseNum = Recvdata[41];
	
			//SEGGER_RTT_printf(0, "RFM300_Heart_Beat %02x%02x%02x%02x%02x%02x  dt:%d pv1:%d pv2:%d pi:%d p1:%d p2:%d ot:%d hr:%d fs:%d pv1low:%d pv2low:%d\n",Senddata[10],Senddata[11],Senddata[12],Senddata[13],Senddata[14],Senddata[15],	
			//				cur_inverter->status.device_Type,cur_inverter->PV1,cur_inverter->PV2,cur_inverter->PI,cur_inverter->Power1,cur_inverter->Power2,cur_inverter->off_times,cur_inverter->heart_rate,cur_inverter->status.function_status,cur_inverter->status.pv1_low_voltage_pritection,cur_inverter->status.pv2_low_voltage_pritection);

			//生成相关的报警报文
			
			create_alarm_record(last_mos_status,last_function_status,last_pv1_low_voltage_pritection,last_pv2_low_voltage_pritection,cur_inverter); 
			return 1;
		}
		else
		{
			TimeOut_times++;
			rt_hw_us_delay(20);
			continue;
		}
			
	}
	return 0;
}

//设备状态设置指令
int RFM300_Status_Init(char *ECUID,char *UID,char Heart_Function,char Device_Type,status_t *status)
{
	unsigned char Status = 0;
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[64] = {'\0'};
	char Recvdata[64] = {'\0'};
	unsigned char last_function_status;	//最后一次功能状态
	inverter_info cur_inverter;

	memcpy(cur_inverter.uid,UID,6);
	cur_inverter.status = *status;
	
	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=0x5A;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=Heart_Function;
	Senddata[17]=Device_Type;
	Senddata[18]=0x00;
	Senddata[19]=0x00;
	Senddata[20]=0x00;
	Senddata[21]=0x00;
	Senddata[22]=0x00;
	Senddata[23]=0x00;
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;
	Senddata[26]=0xFE;
	Senddata[27]=0xFE;
	
#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif
	
	for(i=0;i<2;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xDE)&&
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[10]==Recvdata[10])&&
			(Senddata[11]==Recvdata[11])&&
			(Senddata[12]==Recvdata[12])&&
			(Senddata[13]==Recvdata[13])&&
			(Senddata[14]==Recvdata[14])&&
			(Senddata[15]==Recvdata[15]))
		{
			Status = Recvdata[16];
			if(Status == 1)
			{
				
				last_function_status = status->function_status;
				status->function_status = 1;
				cur_inverter.status.function_status = 1;
				create_alarm_record(cur_inverter.status.comm_failed3_status,last_function_status,cur_inverter.status.pv1_low_voltage_pritection,cur_inverter.status.pv2_low_voltage_pritection,&cur_inverter); 		
			}else
			{
				last_function_status = status->function_status;
				status->function_status = 0;
				cur_inverter.status.function_status = 0;
				create_alarm_record(cur_inverter.status.comm_failed3_status,last_function_status,cur_inverter.status.pv1_low_voltage_pritection,cur_inverter.status.pv2_low_voltage_pritection,&cur_inverter); 
			}
			Status = Recvdata[17];
			if(Status == 1)
			{
				status->device_Type = 1;
			}else
			{
				status->device_Type = 0;
			}
			
			return 1;			
		}
		else
			continue;
	}
	return 0;
}

int RFM300_Set_Uid(char *ECUID,char *UID,int channel,int rate,char *NewUid,char *SaveChannel,char *SaveRate)
{
	int i,check = 0;
	unsigned char RF_leng = 0;
	char Senddata[64] = {'\0'};
	char Recvdata[64] = {'\0'};

	Senddata[0]=0xFB;
	Senddata[1]=0xFB;
	Senddata[2]=0x15;	
	Senddata[3]=0x09;
	Senddata[4]=ECUID[0];
	Senddata[5]=ECUID[1];
	Senddata[6]=ECUID[2];
	Senddata[7]=ECUID[3];
	Senddata[8]=ECUID[4];
	Senddata[9]=ECUID[5];		
	Senddata[10]=UID[0];
	Senddata[11]=UID[1];
	Senddata[12]=UID[2];
	Senddata[13]=UID[3];
	Senddata[14]=UID[4];
	Senddata[15]=UID[5];
	Senddata[16]=channel;
	Senddata[17]=rate;
	Senddata[18]=NewUid[0];
	Senddata[19]=NewUid[1];
	Senddata[20]=NewUid[2];
	Senddata[21]=NewUid[3];
	Senddata[22]=NewUid[4];
	Senddata[23]=NewUid[5];
	
	for(i=3;i<(3+Senddata[2]);i++)
		check=check+Senddata[i];
	Senddata[24] = check/256;
	Senddata[25] = check%256;

	Senddata[26]=0xFE;
	Senddata[27]=0xFE;

#ifdef CMT2300_DEBUG
	for(i=0;i<28;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",Senddata[i]);
	}
	SEGGER_RTT_printf(0, "\n",Senddata[i]);
#endif
	
	for(i=0;i<2;i++){
		SendMessage((unsigned char *)Senddata,28);

		RF_leng = GetMessage((unsigned char *)Recvdata);
		if((RF_leng==28)&&
			(Recvdata[3]==0xD9)&&
			(Senddata[4]==Recvdata[4])&&
			(Senddata[5]==Recvdata[5])&&
			(Senddata[6]==Recvdata[6])&&
			(Senddata[7]==Recvdata[7])&&
			(Senddata[8]==Recvdata[8])&&
			(Senddata[9]==Recvdata[9])&&
			(Senddata[18]==Recvdata[10])&&
			(Senddata[19]==Recvdata[11])&&
			(Senddata[20]==Recvdata[12])&&
			(Senddata[21]==Recvdata[13])&&
			(Senddata[22]==Recvdata[14])&&
			(Senddata[23]==Recvdata[15]))		
		{
				*SaveChannel = Recvdata[16];
				*SaveRate = Recvdata[17];
				SEGGER_RTT_printf(0, "RFM300_Set_Uid  %d %d %d %d %d \n",*SaveChannel,*SaveRate);

				return 1;
		}
		else
				continue;
	}
	return 0;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void commInfo(void)
{
	printdecmsg(ECU_DBG_COMM,"Heart_times",Heart_times);
	printdecmsg(ECU_DBG_COMM,"TimeOut_times",TimeOut_times);
}
FINSH_FUNCTION_EXPORT(commInfo, eg:commInfo());


#endif

