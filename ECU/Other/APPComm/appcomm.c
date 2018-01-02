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
#include "rtc.h"
#include "threadlist.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
static char SendData[MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9] = {'\0'};
extern unsigned char rateOfProgress;
extern ecu_info ecu;

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
	*Command_Id = (RecvData[9]-'0')*1000 + (RecvData[10]-'0')*100 + (RecvData[11]-'0')*10 + (RecvData[12]-'0');;
	return 0;
}

//ECU-RS获取基本信息回应
void APP_Response_BaseInfo(unsigned char *ID,ecu_info curecu,int Length,char * Version,inverter_info *inverter)
{
	int packlength = 0;
	char SIGNAL_LEVEL[4];
	char SIGNAL_CHANNEL[3];
	char Version_length[3];
	int i = 0 , type = inverter[0].status.device_Type;
	for(i=0; (i<MAXINVERTERCOUNT)&&(i < curecu.validNum); i++)
	{
		if(type != inverter[i].status.device_Type)		//判断是否相等
		{
			type = 2;
			break;
		}
	}
	sprintf(SendData,"APS1100000001%s",curecu.ECUID12);
	packlength = 25;
	
	SendData[packlength++] = '0';
	SendData[packlength++] = '2';

	
	SendData[packlength++] = ((unsigned int)(curecu.life_energy*10)/16777216)%256;
	SendData[packlength++] = ((unsigned int)(curecu.life_energy*10)/65536)%256;
	SendData[packlength++] = ((unsigned int)(curecu.life_energy*10)/256)%256;
	SendData[packlength++] =  (unsigned int)(curecu.life_energy*10)%256;

	SendData[packlength++] = (curecu.system_power/16777216)%256;
	SendData[packlength++] = (curecu.system_power/65536)%256;
	SendData[packlength++] = (curecu.system_power/256)%256;
	SendData[packlength++] = curecu.system_power%256;

	SendData[packlength++] = ((unsigned int)(curecu.today_energy * 100)/16777216)%256;
	SendData[packlength++] = ((unsigned int)(curecu.today_energy * 100)/65536)%256;
	SendData[packlength++] = ((unsigned int)(curecu.today_energy * 100)/256)%256;
	SendData[packlength++] = (unsigned int)(curecu.today_energy * 100)%256;
	
	
	
	SendData[packlength++] = curecu.validNum/256;
	SendData[packlength++] = curecu.validNum%256;
	
	SendData[packlength++] = curecu.count/256;
	SendData[packlength++] = curecu.count%256;

	sprintf(SIGNAL_LEVEL,"%03d",curecu.Signal_Level);
	SendData[packlength++] = SIGNAL_LEVEL[0];
	SendData[packlength++] = SIGNAL_LEVEL[1];
	SendData[packlength++] = SIGNAL_LEVEL[2];

	sprintf(SIGNAL_CHANNEL,"%02x",curecu.channel);
	SendData[packlength++] = SIGNAL_CHANNEL[0];
	SendData[packlength++] = SIGNAL_CHANNEL[1];

	SendData[packlength++] = type + '0';

	sprintf(Version_length,"%03d",Length);
	SendData[packlength++] = Version_length[0];
	SendData[packlength++] = Version_length[1];
	SendData[packlength++] = Version_length[2];
	
	sprintf(&SendData[packlength],"%s",Version);
	packlength += Length;

	
	
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

//ECU-RS系统信息回应   mapflag   0表示匹配成功  1 表示匹配不成功
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int validNum)
{
	inverter_info *curinverter = inverter;
	unsigned short inverter_length = 0;
	unsigned int Energy;
	char curTime[15] = {'\0'};
	unsigned char inverter_data[58] = {'\0'};
	int i = 0;
	int length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	memcpy(curTime,ecu.JsonTime,14);
	if(mapflag == 1)	//匹配失败，发送失败命令
	{
		sprintf(SendData,"APS110015000201\n");
		SendToSocketA(SendData ,16,ID);
		return;
	}else{				//匹配成功，发送成功命令

		if(memcmp(curTime,"00000000000000",14))
		{
			sprintf(SendData,"APS110015000200");   //13字节

			SendData[15] = '0';	
			SendData[16] = '2';


			SendData[17] = validNum/256;	
			SendData[18] = validNum%256;

			SendData[19] = (curTime[0] - '0')*16+(curTime[1] - '0');
			SendData[20] = (curTime[2] - '0')*16+(curTime[3] - '0');
			SendData[21] = (curTime[4] - '0')*16+(curTime[5] - '0');
			SendData[22] = (curTime[6] - '0')*16+(curTime[7] - '0');
			SendData[23] = (curTime[8] - '0')*16+(curTime[9] - '0');
			SendData[24] = (curTime[10] - '0')*16+(curTime[11] - '0');
			SendData[25] = (curTime[12] - '0')*16+(curTime[13] - '0');
		
			length = 26;
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
				
				inverter_data[21] = (int)curinverter->AveragePower1/256;
				inverter_data[22] = (int)curinverter->AveragePower1%256;
				inverter_data[23] = (int)curinverter->AveragePower2/256;
				inverter_data[24] = (int)curinverter->AveragePower2%256;
				
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
		}else
		{
			sprintf(SendData,"APS110015000202\n");
			SendToSocketA(SendData ,16,ID);	
			return;
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
			sprintf(SendData,"APS110015000202\n");
			SendToSocketA(SendData ,16,ID);			
		}
		
#if 1
	for(i=0;i<length+9;i++)
	{
		SEGGER_RTT_printf(0, "%02x ",SendData[i]);
	}
	SEGGER_RTT_printf(0, "\n");
#endif	
		
	}


}

void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date)
{
	int packlength = 0,length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000301\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	//拼接需要发送的报文
	sprintf(SendData,"APS110015000300");
	packlength = 15;
	
	read_system_power(date,&SendData[15],&length);
	packlength += length;
	
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
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char request_type)
{
	int packlength = 0,len_body = 0;
	char date_time[15] = { '\0' };
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	apstime(date_time);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000401\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	sprintf(SendData,"APS110015000400");
	packlength = 15;
	//拼接需要发送的报文
	if(request_type == '0')
	{//最近一周
		SendData[packlength++] = '0';
		SendData[packlength++] = '0';
		
		read_weekly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '1')
	{//最近一个月
		SendData[packlength++] = '0';
		SendData[packlength++] = '1';
		read_monthly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == '2')
	{//最近一年
		SendData[packlength++] = '0';
		SendData[packlength++] = '2';
		read_yearly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
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



//ECU-RS设置组网回应
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"APS1100150005%02d\n",result);
	SendToSocketA(SendData ,16,ID);
}

//06 COMMAND_SETTIME						//时间设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetTime(unsigned char *ID,char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150006%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,ID);
}

//07 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150007%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,ID);
}

//08 查看ECU当前硬件状态
void APP_Response_GetECUHardwareStatus(unsigned char *ID,unsigned char mapping)
{
	int packlength = 0;
	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110120000800%02d",WIFI_MODULE_TYPE);
		memset(&SendData[17],'0',100);
		SendData[117] = 'E';
		SendData[118] = 'N';
		SendData[119] = 'D';
		SendData[120] = '\n';
		packlength = 121;
	}else
	{
		sprintf(SendData,"APS110015000801\n");
		packlength = 16;
	}
	SendToSocketA(SendData ,packlength,ID);
}


//ECU-RS设置WIFI密码
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS1100150010%02d\n",result);
	SendToSocketA(SendData ,16,ID);
}

//11	AP密码设置请求
void APP_Response_GetIDInfo(char mapping,unsigned char *ID,inverter_info *inverter)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	char uid[7];
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110015001100");
		packlength = 15;
		for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); index++, curinverter++)
		{
			
			uid[0] = (curinverter->uid[0] - '0')*16+(curinverter->uid[1] - '0');
			uid[1] = (curinverter->uid[2] - '0')*16+(curinverter->uid[3] - '0');
			uid[2] = (curinverter->uid[4] - '0')*16+(curinverter->uid[5] - '0');
			uid[3] = (curinverter->uid[6] - '0')*16+(curinverter->uid[7] - '0');
			uid[4] = (curinverter->uid[8] - '0')*16+(curinverter->uid[9] - '0');
			uid[5] = (curinverter->uid[10] - '0')*16+(curinverter->uid[11] - '0');
			memcpy(&SendData[packlength],uid,6);	
			packlength += 6;
		}
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';

		
	}else
	{
		sprintf(SendData,"APS110015001101\n");
		packlength = 16;
	}		
	SendToSocketA(SendData ,packlength,ID);

}

void APP_Response_GetTime(char mapping,unsigned char *ID,char *Time)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110032001200%sEND\n",Time);
		packlength = 33;
	}else
	{
		sprintf(SendData,"APS110015001201\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength,ID);

}

//ECU-RS剩余存储空间
void APP_Response_FlashSize(char mapping,unsigned char *ID,unsigned int Flashsize)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110000001300");
		packlength = 15;
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
		sprintf(SendData,"APS110015001301\n");
		packlength = 16;
	}	
	

}

//14 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GetWiredNetwork(char mapping,unsigned char *ID,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr,char *MacAddress)
{
	int packlength = 0;
	char MAC[13] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	//拼接需要发送的报文
	
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100150014%02d\n",mapping);
		packlength = 16;
		SendToSocketA(SendData ,packlength,ID);
	}else
	{
		sprintf(SendData,"APS1100150014%02d\n",mapping);
		packlength = 15;
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



//ECU-RS设置信道回应
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapflag == 1)
	{
		sprintf(SendData,"APS110015001501\n");
		SendToSocketA(SendData ,16,ID);
	}else{
		sprintf(SendData,"APS110023001500%02x%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);		
		SendToSocketA(SendData ,24,ID);
	}
}




void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS1100150016%02d\n",result);
	SendToSocketA(SendData ,16,ID);	
}

void APP_Response_GetRSDHistoryInfo(char mapping,unsigned char *ID,char *date_time ,char * UID)
{
	int packlength = 0,len_body = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015001701\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,ID);
		return ;
	}

	//匹配成功
	sprintf(SendData,"APS110015001700%s",date_time);
	packlength = 23;
	memcpy(&SendData[packlength],UID,6);
	packlength += 6;
	read_RSD_info(date_time,UID,&SendData[packlength],&len_body);
	packlength += len_body;
	
	//没有Body部分数据
	if(len_body == 0)
	{
		sprintf(SendData,"APS110015001702\n");
		packlength = 16;
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

void APP_Response_GetShortAddrInfo(char mapping,unsigned char *ID,inverter_info *inverter)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	char uid[7];
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110015001800");
		packlength = 15;
		SendData[packlength++] = rateOfProgress;
		for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); index++, curinverter++)
		{
			
			uid[0] = (curinverter->uid[0] - '0')*16+(curinverter->uid[1] - '0');
			uid[1] = (curinverter->uid[2] - '0')*16+(curinverter->uid[3] - '0');
			uid[2] = (curinverter->uid[4] - '0')*16+(curinverter->uid[5] - '0');
			uid[3] = (curinverter->uid[6] - '0')*16+(curinverter->uid[7] - '0');
			uid[4] = (curinverter->uid[8] - '0')*16+(curinverter->uid[9] - '0');
			uid[5] = (curinverter->uid[10] - '0')*16+(curinverter->uid[11] - '0');
			memcpy(&SendData[packlength],uid,6);	
			packlength += 6;
			SendData[packlength++] = curinverter->shortaddr/256;
			SendData[packlength++] = curinverter->shortaddr%256;
		}
		
		SendData[packlength++] = 'E';
		SendData[packlength++] = 'N';
		SendData[packlength++] = 'D';
		
		SendData[5] = (packlength/1000) + '0';
		SendData[6] = ((packlength/100)%10) + '0';
		SendData[7] = ((packlength/10)%10) + '0';
		SendData[8] = ((packlength)%10) + '0';
		SendData[packlength++] = '\n';

		
	}else
	{
		sprintf(SendData,"APS110015001801\n");
		packlength = 16;
	}		
	SendToSocketA(SendData ,packlength,ID);

}



