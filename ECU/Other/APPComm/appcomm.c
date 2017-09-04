/*****************************************************************************/
/* File      : appcomm.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-08 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "appcomm.h"
#include "string.h"
#include "SEGGER_RTT.h"
#include "usart5.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
static char SendData[MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9] = {'\0'};


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//解析收到的数据
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id)
{
	//APS
	if(strncmp(RecvData, "APS", 3))
		return -1;
	//版本号 
	//长度
	*Data_Len = packetlen((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*10 + (RecvData[10]-'0');
	return 0;
}

//ECU-RS获取基本信息回应
void APP_Response_BaseInfo(unsigned char *ID,char *ECU_NO,char *TYPE,char SIGNAL_LEVEL,char *SIGNAL_CHANNEL,int Length,char * Version,inverter_info *inverter,int validNum)
{
	int i = 0 , type = inverter[0].status.device_Type;
	for(i=0; (i<MAXINVERTERCOUNT)&&(i < validNum); i++)
	{
		if(type != inverter[i].status.device_Type)		//判断是否相等
		{
			type = 2;
			break;
		}
	}
	
	
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"a00000000APS11%04d01%s%03d%03d%s%1d%02d%sEND\n",(37+Length),ECU_NO,101,SIGNAL_LEVEL,SIGNAL_CHANNEL,type,Length,Version);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];
	SEGGER_RTT_printf(0, "APP_Response_BaseInfo %s\n",&SendData[9]);
	WIFI_SendData(SendData, (47+Length));
}

//ECU-RS系统信息回应   mapflag   0表示匹配成功  1 表示匹配不成功
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int validNum)
{
	inverter_info *curinverter = inverter;
	unsigned short inverter_length = 0;
	
	unsigned char inverter_data[23] = {'\0'};
	int i = 0;
	int length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	//SEGGER_RTT_printf(0, "SystemInfo %d %d  %d\n",mapflag,validNum,(MAXINVERTERCOUNT*INVERTERLENGTH + 16));

	if(mapflag == 1)	//匹配失败，发送失败命令
	{
		sprintf(SendData,"a00000000APS1100130201\n");
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];
		WIFI_SendData(SendData, 23);
		return;
	}else{				//匹配成功，发送成功命令
		
		sprintf(SendData,"a00000000APS1100131200");   //13字节
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];

		SendData[22] = validNum/256;	
		SendData[23] = validNum%256;

		length = 24;
		for(i=0; (i<MAXINVERTERCOUNT)&&(i < validNum); i++)			
		{
			memset(inverter_data,0x00,23);
			if(curinverter->status.device_Type == 0)		//开关设备
			{
				inverter_length = 13;
				//拼接13字节数据包
				memcpy(&inverter_data[0],curinverter->uid,6);
				inverter_data[6] = curinverter->status.device_Type;
				
				inverter_data[7] |=  curinverter->status.mos_status;
				inverter_data[7] |=  (curinverter->status.function_status << 1);

				inverter_data[8] = curinverter->heart_rate /256;
				inverter_data[9] = curinverter->heart_rate %256;

				inverter_data[10] = curinverter->off_times/256;
				inverter_data[11] = curinverter->off_times%256;

				inverter_data[12] = curinverter->restartNum;
				
			}else if(curinverter->status.device_Type == 1) 	//监控设备
			{
				inverter_length = 22;
				//拼接20字节数据包
				memcpy(&inverter_data[0],curinverter->uid,6);
				inverter_data[6] = curinverter->status.device_Type;
				
				inverter_data[7] |=  curinverter->status.mos_status;
				inverter_data[7] |=  (curinverter->status.function_status << 1);
				inverter_data[7] |=  (curinverter->status.pv1_low_voltage_pritection<< 2);
				inverter_data[7] |=  (curinverter->status.pv2_low_voltage_pritection << 3);

				inverter_data[8] = curinverter->heart_rate /256;
				inverter_data[9] = curinverter->heart_rate %256;

				inverter_data[10] = curinverter->off_times/256;
				inverter_data[11] = curinverter->off_times%256;
				
				inverter_data[12] = curinverter->restartNum;

				inverter_data[13] = curinverter->PV1/256;
				inverter_data[14] = curinverter->PV1%256;
				inverter_data[15] = curinverter->PV2/256;
				inverter_data[16] = curinverter->PV2%256;
				inverter_data[17] = curinverter->PI;
				inverter_data[18] = curinverter->Power1/256;
				inverter_data[19] = curinverter->Power1%256;
				inverter_data[20] = curinverter->Power2/256;
				inverter_data[21] = curinverter->Power2%256;
				
			}
			

			memcpy(&SendData[length],inverter_data,inverter_length);
			length += inverter_length;
			
			curinverter++;
		}

		if(validNum > 0)
		{		
			length = length - 9 + 3;
			
			//改变报文字节长度
			SendData[14] = (length/1000) + '0';
			SendData[15] =	((length/100)%10) + '0';
			SendData[16] = ((length/10)%10) + '0';
			SendData[17] = ((length)%10) + '0';
			SendData[length-3+9] = 'E';
			SendData[length-2+9] = 'N';
			SendData[length-1+9] = 'D';
			SendData[length+9] = '\n';
		}else
		{
			length = 13;
			SendData[length+9] = '\n';
			
		}
		
#if 0
	for(i=0;i<length+9;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",SendData[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif	
		
		WIFI_SendData(SendData, (length+10));
	}


}

//ECU-RS设置组网回应
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"a00000000APS11001303%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];	
	WIFI_SendData(SendData, 23);
}

//ECU-RS设置信道回应
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char *SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapflag == 1)
	{
		sprintf(SendData,"a00000000APS1100130401\n");
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];		
		WIFI_SendData(SendData, 23);
	}else{
		sprintf(SendData,"a00000000APS1100210400%s%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);
		SendData[1] = ID[0];
		SendData[2] = ID[1];
		SendData[3] = ID[2];
		SendData[4] = ID[3];
		SendData[5] = ID[4];
		SendData[6] = ID[5];
		SendData[7] = ID[6];
		SendData[8] = ID[7];		
		WIFI_SendData(SendData, 31);
	}
}

//ECU-RS设置WIFI密码
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"a00000000APS11001305%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];		
	WIFI_SendData(SendData, 23);
}


void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"a00000000APS11001306%02d\n",result);
	SendData[1] = ID[0];
	SendData[2] = ID[1];
	SendData[3] = ID[2];
	SendData[4] = ID[3];
	SendData[5] = ID[4];
	SendData[6] = ID[5];
	SendData[7] = ID[6];
	SendData[8] = ID[7];		
	WIFI_SendData(SendData, 23);
}

