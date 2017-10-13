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
#include "stdio.h"
#include "serverfile.h"

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
	*Data_Len = packetlen_A((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*10 + (RecvData[10]-'0');
	return 0;
}

//ECU-RS获取基本信息回应
void APP_Response_BaseInfo(unsigned char *ID,char *ECU_NO,char *TYPE,char SIGNAL_LEVEL,char SIGNAL_CHANNEL,int Length,char * Version,inverter_info *inverter,int validNum)
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
	sprintf(SendData,"APS11%04d01%s%03d%03d%02x%1d%02d%sEND\n",(37+Length),ECU_NO,101,SIGNAL_LEVEL,SIGNAL_CHANNEL,type,Length,Version);
	SEGGER_RTT_printf(0, "APP_Response_BaseInfo %s\n",SendData);
	SendToSocketA(SendData ,(38+Length),ID);
}

//ECU-RS系统信息回应   mapflag   0表示匹配成功  1 表示匹配不成功
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int validNum)
{
	inverter_info *curinverter = inverter;
	unsigned short inverter_length = 0;
	unsigned int Energy;
	unsigned char inverter_data[58] = {'\0'};
	int i = 0;
	int length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	if(mapflag == 1)	//匹配失败，发送失败命令
	{
		sprintf(SendData,"APS1100130201\n");
		SendToSocketA(SendData ,14,ID);
		return;
	}else{				//匹配成功，发送成功命令
		
		sprintf(SendData,"APS1100130200");   //13字节

		SendData[13] = validNum/256;	
		SendData[14] = validNum%256;

		length = 15;
		for(i=0; (i<MAXINVERTERCOUNT)&&(i < validNum); i++)			
		{
			memset(inverter_data,0x00,58);

			inverter_length = 57;
			//拼接57字节数据包
			inverter_data[0] = (curinverter->uid[0] - '0')*0x10 + (curinverter->uid[1] - '0');
			inverter_data[1] = (curinverter->uid[2] - '0')*0x10 + (curinverter->uid[3] - '0');
			inverter_data[2] = (curinverter->uid[4] - '0')*0x10 + (curinverter->uid[5] - '0');
			inverter_data[3] = (curinverter->uid[6] - '0')*0x10 + (curinverter->uid[7] - '0');
			inverter_data[4] = (curinverter->uid[8] - '0')*0x10 + (curinverter->uid[9] - '0');
			inverter_data[5] = (curinverter->uid[10] - '0')*0x10 + (curinverter->uid[11] - '0');

			inverter_data[6] = curinverter->status.device_Type;
				
			inverter_data[7] |=  curinverter->status.comm_failed3_status;
			inverter_data[7] |=  (curinverter->status.function_status << 1);
			inverter_data[7] |=  (curinverter->status.pv1_low_voltage_pritection << 2);
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
			inverter_data[17] = curinverter->PI/256;
			inverter_data[18] = curinverter->PI%256;
			inverter_data[19] = curinverter->PI2/256;
			inverter_data[20] = curinverter->PI2%256;
			
			inverter_data[21] = curinverter->Power1/256;
			inverter_data[22] = curinverter->Power1%256;
			inverter_data[23] = curinverter->Power2/256;
			inverter_data[24] = curinverter->Power2%256;
			
			inverter_data[25] = curinverter->PV_Output/256;
			inverter_data[26] = curinverter->PV_Output%256;
			inverter_data[27] = curinverter->PI_Output/256;
			inverter_data[28] = curinverter->PI_Output%256;
			inverter_data[29] = curinverter->Power_Output/256;
			inverter_data[30] = curinverter->Power_Output%256;

			inverter_data[31] = curinverter->RSSI;
			
			Energy = curinverter->EnergyPV1/36;
			inverter_data[32] = (Energy/16777216)%256;
			inverter_data[33] = (Energy/65536)%256;
			inverter_data[34] = (Energy/256)%256;
			inverter_data[35] = Energy%256;
			
			Energy = curinverter->EnergyPV2/36;
			inverter_data[36] = (Energy/16777216)%256;
			inverter_data[37] = (Energy/65536)%256;
			inverter_data[38] = (Energy/256)%256;
			inverter_data[39] = Energy%256;

			Energy = curinverter->EnergyPV_Output/36;
			inverter_data[40] = (Energy/16777216)%256;
			inverter_data[41] = (Energy/65536)%256;
			inverter_data[42] = (Energy/256)%256;
			inverter_data[43] = Energy%256;

			inverter_data[44] = curinverter->Mos_CloseNum;
			inverter_data[45] = curinverter->version/256;
			inverter_data[46] = curinverter->version%256;
			memcpy(&SendData[length],inverter_data,inverter_length);
			length += inverter_length;
			
			curinverter++;
		}

		if(validNum > 0)
		{		
			SendData[length++] = 'E';
			SendData[length++] = 'N';
			SendData[length++] = 'D';
			//改变报文字节长度
			SendData[5] = (length/1000) + '0';
			SendData[6] =	((length/100)%10) + '0';
			SendData[7] = ((length/10)%10) + '0';
			SendData[8] = ((length)%10) + '0';
			
			SendData[length] = '\n';
			SendToSocketA(SendData ,length+1,ID);
		}else
		{
			length = 13;
			SendData[length] = '\n';
			SendToSocketA(SendData ,length+1,ID);
			
		}
		
#if 0
	for(i=0;i<length+9;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",SendData[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif	
		
	}


}

//ECU-RS设置组网回应
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"APS11001303%02d\n",result);
	SendToSocketA(SendData ,14,ID);
}

//ECU-RS设置信道回应
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapflag == 1)
	{
		sprintf(SendData,"APS1100130401\n");
		SendToSocketA(SendData ,14,ID);
	}else{
		sprintf(SendData,"APS1100210400%02x%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);		
		SendToSocketA(SendData ,22,ID);
	}
}

//ECU-RS设置WIFI密码
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS11001305%02d\n",result);
	SendToSocketA(SendData ,14,ID);
}


void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS11001306%02d\n",result);
	SendToSocketA(SendData ,14,ID);	
}

void APP_Response_GetRSDHistoryInfo(char mapping,unsigned char *ID,char *date_time ,char * UID)
{
	int packlength = 0,len_body = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130701\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	//匹配成功
	sprintf(SendData,"APS1100150700%s",date_time);
	packlength = 21;
	memcpy(&SendData[packlength],UID,6);
	packlength += 6;
	read_RSD_info(date_time,UID,&SendData[packlength],&len_body);
	packlength += len_body;
	
	//没有Body部分数据
	if(len_body == 0)
	{
		sprintf(SendData,"APS1100130702\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength,ID);
	

}


//08 COMMAND_GENERATIONCURVE		//发电量曲线请求    mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char *date_time ,char request_type)
{
	int packlength = 0,len_body = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100130801\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}
	//匹配成功

	sprintf(SendData,"APS1100150800%s",date_time);
	packlength = 21;
	//拼接需要发送的报文
	if(request_type == '0')
	{
		//收到日期去获取当月和上一个月各天的发电量
		SendData[packlength++] = '0';
		SendData[packlength++] = '0';
		
		read_monthly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '1')
	{
		//收到日期去获取当年和前一年的各月的发电量
		SendData[packlength++] = '0';
		SendData[packlength++] = '1';
		read_yearly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '2')
	{
		//历史上各年发电量
		SendData[packlength++] = '0';
		SendData[packlength++] = '2';
		read_history_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}

	//时间点不存在时直接返回02
	if(len_body == 0)
	{
		sprintf(SendData,"APS1100130802\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength,ID);
}



//09 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS11001309%02d\n",mapping);
	packlength = 14;
	
	SendToSocketA(SendData ,packlength,ID);
}

//10 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GetWiredNetwork(char mapping,unsigned char *ID,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr,char *MacAddress)
{
	int packlength = 0;
	char MAC[13] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	//拼接需要发送的报文
	
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS11001510%02d\n",mapping);
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
	}else
	{
		sprintf(SendData,"APS11001510%02d\n",mapping);
		packlength = 13;
		if(dhcpStatus == 0)
		{
			SendData[packlength++] = '0';
			SendData[packlength++] = '0';
		}else
		{
			SendData[packlength++] = '0';
			SendData[packlength++] = '1';
		}
		SendData[packlength++] = IPAddr.IP1;
		SendData[packlength++] = IPAddr.IP2;
		SendData[packlength++] = IPAddr.IP3;
		SendData[packlength++] = IPAddr.IP4;
				
		SendData[packlength++] = MSKAddr.IP1;
		SendData[packlength++] = MSKAddr.IP2;
		SendData[packlength++] = MSKAddr.IP3;
		SendData[packlength++] = MSKAddr.IP4;
		
		SendData[packlength++] = GWAddr.IP1;
		SendData[packlength++] = GWAddr.IP2;
		SendData[packlength++] = GWAddr.IP3;
		SendData[packlength++] = GWAddr.IP4;

		SendData[packlength++] = DNS1Addr.IP1;
		SendData[packlength++] = DNS1Addr.IP2;
		SendData[packlength++] = DNS1Addr.IP3;
		SendData[packlength++] = DNS1Addr.IP4;

		SendData[packlength++] = DNS2Addr.IP1;
		SendData[packlength++] = DNS2Addr.IP2;
		SendData[packlength++] = DNS2Addr.IP3;
		SendData[packlength++] = DNS2Addr.IP4;

		sprintf(MAC,"%02x%02x%02x%02x%02x%02x",MacAddress[0],MacAddress[1],MacAddress[2],MacAddress[3],MacAddress[4],MacAddress[5]);
		memcpy(&SendData[packlength],MAC,12);
		packlength += 12;

		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';

		
		SendToSocketA(SendData ,packlength,ID);
	}
	
	
}



//ECU-RS剩余存储空间
void APP_Response_FlashSize(char mapping,unsigned char *ID,unsigned int Flashsize)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS1100001100");
		packlength = 13;
		SendData[packlength++] = (Flashsize/16777216)%256;
		SendData[packlength++] = (Flashsize/65536)%256;
		SendData[packlength++] = (Flashsize/256)%256;
		SendData[packlength++] = Flashsize%256;
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';
		SendToSocketA(SendData ,packlength,ID);
	}else
	{
		sprintf(SendData,"APS1100131101\n");
		packlength = 14;
	}	
	

}

void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date)
{
	int packlength = 0,length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100131201\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	//拼接需要发送的报文
	sprintf(SendData,"APS1100131200");
	packlength = 13;
	
	read_system_power(date,&SendData[13],&length);
	packlength += length;
	if(length == 8)
	{	//无需上传数据  设置为02
		sprintf(SendData,"APS1100131202\n");
		packlength = 14;
		SendToSocketA(SendData ,packlength,ID);
	}
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength,ID);

}


