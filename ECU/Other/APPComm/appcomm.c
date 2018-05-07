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
#include "third_inverter.h"
#include "stdlib.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
static char SendData[MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9] = {'\0'};
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern inverter_third_info thirdInverterInfo[MAX_THIRD_INVERTER_COUNT];
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

int Resolve_Server(ECUServerInfo_t *serverInfo)
{
	//如果serverCmdType 为获取指令，解析对应文件到结构体中
	char IP_Str[20] = {'\0'};
	char temp[4] = {'\0'};
	unsigned char IP_Str_site = 0;
	unsigned char site = 0;
	int i = 0;
	FILE *fp;
	char *buff = NULL;
	buff = malloc(512);
	memset(buff,'\0',512);
	if(serverInfo->serverCmdType == SERVER_UPDATE_GET)
	{
		//初始化电量上传参数
		fp = fopen("/config/FTPADD.CON", "r");
		if(fp)
		{
			while(1)
			{
				memset(buff, '\0', 512);
				fgets(buff, 512, fp);
				if(!strlen(buff))
					break;
				if(!strncmp(buff, "Domain", 6))
				{
					memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
					strcpy(serverInfo->domain, &buff[7]);
					if('\n' == serverInfo->domain[strlen(serverInfo->domain)-1])
					{
						serverInfo->domain[strlen(serverInfo->domain)-1] = '\0';
					}
						
				}
				if(!strncmp(buff, "IP", 2))
				{
					strcpy(IP_Str, &buff[3]);
					if('\n' == IP_Str[strlen(IP_Str)-1])
						IP_Str[strlen(IP_Str)-1] = '\0';
					//解析到serverInfo中去
					for(i = 0;i<(strlen(IP_Str)+1);i++)
					{
						if((IP_Str[i] =='.') ||(IP_Str[i] =='\0'))
						{
							memset(temp,'\0',4);
							memcpy(temp,&IP_Str[IP_Str_site],i-IP_Str_site);
							serverInfo->IP[site] = atoi(temp);
							IP_Str_site = i+1;
							site++;
						}
					}
					
				}
				if(!strncmp(buff, "Port", 4))
					serverInfo->Port1=atoi(&buff[5]);

				serverInfo->Port2=0;

			}
			fclose(fp);
		}
		else
		{
			memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
			memcpy(serverInfo->domain,UPDATE_SERVER_DOMAIN,strlen(UPDATE_SERVER_DOMAIN));
			serverInfo->IP[0] = 60;
			serverInfo->IP[1] = 190;
			serverInfo->IP[2] = 131;
			serverInfo->IP[3] = 190;
			serverInfo->Port1 = UPDATE_SERVER_PORT1;
			serverInfo->Port2 = 0;			
		}
	}else if(serverInfo->serverCmdType == SERVER_CLIENT_GET)
	{
		//初始化电量上传参数
		fp = fopen("/config/datacent.con", "r");
		if(fp)
		{
			while(1)
			{
				memset(buff, '\0', 512);
				fgets(buff, 512, fp);
				if(!strlen(buff))
					break;
				if(!strncmp(buff, "Domain", 6))
				{
					memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
					strcpy(serverInfo->domain, &buff[7]);
					if('\n' == serverInfo->domain[strlen(serverInfo->domain)-1])
					{
						serverInfo->domain[strlen(serverInfo->domain)-1] = '\0';
					}
						
				}
				if(!strncmp(buff, "IP", 2))
				{
					strcpy(IP_Str, &buff[3]);
					if('\n' == IP_Str[strlen(IP_Str)-1])
						IP_Str[strlen(IP_Str)-1] = '\0';
					//解析到serverInfo中去
					for(i = 0;i<(strlen(IP_Str)+1);i++)
					{
						if((IP_Str[i] =='.') ||(IP_Str[i] =='\0'))
						{
							memset(temp,'\0',4);
							memcpy(temp,&IP_Str[IP_Str_site],i-IP_Str_site);
							serverInfo->IP[site] = atoi(temp);
							IP_Str_site = i+1;
							site++;
						}
					}
					
				}
				if(!strncmp(buff, "Port1", 5))
					serverInfo->Port1=atoi(&buff[6]);
				if(!strncmp(buff, "Port2", 5))
					serverInfo->Port2=atoi(&buff[6]);
			}
			fclose(fp);	
		}
		else
		{
			memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
			memcpy(serverInfo->domain,CLIENT_SERVER_DOMAIN,strlen(CLIENT_SERVER_DOMAIN));
			serverInfo->IP[0] = 60;
			serverInfo->IP[1] = 190;
			serverInfo->IP[2] = 131;
			serverInfo->IP[3] = 190;
			serverInfo->Port1 = CLIENT_SERVER_PORT1;
			serverInfo->Port2 = CLIENT_SERVER_PORT2;			
		}
		
	}else if(serverInfo->serverCmdType == SERVER_CONTROL_GET)
	{
		//初始化电量上传参数
		fp = fopen("/config/CONTROL.CON", "r");
		if(fp)
		{
			while(1)
			{
				memset(buff, '\0', 512);
				fgets(buff, 512, fp);
				if(!strlen(buff))
					break;
				if(!strncmp(buff, "Domain", 6))
				{
					memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
					strcpy(serverInfo->domain, &buff[7]);
					if('\n' == serverInfo->domain[strlen(serverInfo->domain)-1])
					{
						serverInfo->domain[strlen(serverInfo->domain)-1] = '\0';
					}
						
				}
				if(!strncmp(buff, "IP", 2))
				{
					strcpy(IP_Str, &buff[3]);
					if('\n' == IP_Str[strlen(IP_Str)-1])
						IP_Str[strlen(IP_Str)-1] = '\0';
					//解析到serverInfo中去
					for(i = 0;i<(strlen(IP_Str)+1);i++)
					{
						if((IP_Str[i] =='.') ||(IP_Str[i] =='\0'))
						{
							memset(temp,'\0',4);
							memcpy(temp,&IP_Str[IP_Str_site],i-IP_Str_site);
							serverInfo->IP[site] = atoi(temp);
							IP_Str_site = i+1;
							site++;
						}
					}
					
				}
				if(!strncmp(buff, "Port1", 5))
					serverInfo->Port1=atoi(&buff[6]);
				if(!strncmp(buff, "Port2", 5))
					serverInfo->Port2=atoi(&buff[6]);

			}
			fclose(fp);
		}
		else
		{
			memset(serverInfo->domain,'\0',sizeof(serverInfo->domain)/sizeof(serverInfo->domain[0]));
			memcpy(serverInfo->domain,CONTROL_SERVER_DOMAIN,strlen(CONTROL_SERVER_DOMAIN));
			serverInfo->IP[0] = 60;
			serverInfo->IP[1] = 190;
			serverInfo->IP[2] = 131;
			serverInfo->IP[3] = 190;
			serverInfo->Port1 = CONTROL_SERVER_PORT1;
			serverInfo->Port2 = CONTROL_SERVER_PORT2;			
		}
	}

	free(buff);
	buff = NULL;
	
	return 0;
	
}


//ECU-RS获取基本信息回应
void APP_Response_BaseInfo(ecu_info curecu,int Length,char * Version,inverter_info *inverter)
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
	SendToSocketA(SendData ,packlength);

}

//ECU-RS系统信息回应   mapflag   0表示匹配成功  1 表示匹配不成功
void APP_Response_SystemInfo(unsigned char mapflag,inverter_info *inverter,int validNum)
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
		SendToSocketA(SendData ,16);
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
				inverter_data[47] = curinverter->model;
				inverter_data[48] = curinverter->temperature;
				memcpy(&SendData[length],inverter_data,inverter_length);
				length += inverter_length;
				
				curinverter++;
			}
		}else
		{
			sprintf(SendData,"APS110015000202\n");
			SendToSocketA(SendData ,16);	
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
			SendToSocketA(SendData ,length+1);
		}else
		{
			sprintf(SendData,"APS110015000202\n");
			SendToSocketA(SendData ,16);			
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

void APP_Response_PowerCurve(char mapping,char * date)
{
	int packlength = 0,length = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000301\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
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
	
	SendToSocketA(SendData ,packlength);

}

//08 COMMAND_GENERATIONCURVE		//发电量曲线请求    mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GenerationCurve(char mapping,char request_type)
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
		SendToSocketA(SendData ,packlength);
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
	
	SendToSocketA(SendData ,packlength);

}



//ECU-RS设置组网回应
void APP_Response_SetNetwork(unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"APS1100150005%02d\n",result);
	SendToSocketA(SendData ,16);
}

//06 COMMAND_SETTIME						//时间设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetTime(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150006%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}

//07 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWiredNetwork(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150007%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength);
}

//08 查看ECU当前硬件状态
void APP_Response_GetECUHardwareStatus(unsigned char mapping)
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
	SendToSocketA(SendData ,packlength);
}


//ECU-RS设置WIFI密码
void APP_Response_SetWifiPassword(unsigned char result)
{
	//char SendData[20] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS1100150010%02d\n",result);
	SendToSocketA(SendData ,16);
}

//11	AP密码设置请求
void APP_Response_GetIDInfo(char mapping,inverter_info *inverter)
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
	SendToSocketA(SendData ,packlength);

}

void APP_Response_GetTime(char mapping,char *Time)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110032001200%sEND\n",Time);
		packlength = 33;
	}else
	{
		sprintf(SendData,"APS110015001201\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);

}

//ECU-RS剩余存储空间
void APP_Response_FlashSize(char mapping,unsigned int Flashsize)
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
		SendToSocketA(SendData ,packlength);
	}else
	{
		sprintf(SendData,"APS110015001301\n");
		packlength = 16;
	}	
	

}

//14 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GetWiredNetwork(char mapping,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr,char *MacAddress)
{
	int packlength = 0;
	char MAC[13] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	//拼接需要发送的报文
	
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS1100150014%02d\n",mapping);
		packlength = 16;
		SendToSocketA(SendData ,packlength);
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

		
		SendToSocketA(SendData ,packlength);
	}
	
	
}



//ECU-RS设置信道回应
void APP_Response_SetChannel(unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL)
{
	//char SendData[22] = {'\0'};
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(mapflag == 1)
	{
		sprintf(SendData,"APS110015001501\n");
		SendToSocketA(SendData ,16);
	}else{
		sprintf(SendData,"APS110023001500%02x%03dEND\n",SIGNAL_CHANNEL,SIGNAL_LEVEL);		
		SendToSocketA(SendData ,24);
	}
}




void APP_Response_IOInitStatus(unsigned char result)
{
	unsigned char inverter_data[8] = {'\0'};
	int i =0,length = 0,inverter_length =0;
	
	//char SendData[20] = {'\0'};
	if((result == 0)||(result == 1))
	{
		memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
		sprintf(SendData,"APS1100150016%02d\n",result);
		SendToSocketA(SendData ,16);	
	}else if(result == 2)
	{
		memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
		if(memcmp(ecu.JsonTime,"00000000000000",14))
		{
			sprintf(SendData,"APS110015001600");   //13字节

		
			length = 15;
			for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)			
			{
				memset(inverter_data,0x00,8);

				inverter_length = 7;
				//拼接57字节数据包
				inverter_data[0] = (inverterInfo[i].uid[0] - '0')*0x10 + (inverterInfo[i].uid[1] - '0');
				inverter_data[1] = (inverterInfo[i].uid[2] - '0')*0x10 + (inverterInfo[i].uid[3] - '0');
				inverter_data[2] = (inverterInfo[i].uid[4] - '0')*0x10 + (inverterInfo[i].uid[5] - '0');
				inverter_data[3] = (inverterInfo[i].uid[6] - '0')*0x10 + (inverterInfo[i].uid[7] - '0');
				inverter_data[4] = (inverterInfo[i].uid[8] - '0')*0x10 + (inverterInfo[i].uid[9] - '0');
				inverter_data[5] = (inverterInfo[i].uid[10] - '0')*0x10 + (inverterInfo[i].uid[11] - '0');


				if(memcmp( inverterInfo[i].LastCollectTime,"00000000000000",14))
				{
					inverter_data[6] = inverterInfo[i].status.function_status;
				}else
				{
					inverter_data[6] = 0x02;
				}
				
					
				
				memcpy(&SendData[length],inverter_data,inverter_length);
				length += inverter_length;
			}
		}else
		{
			sprintf(SendData,"APS110018001600END\n");
			SendToSocketA(SendData ,19);	
			return;
		}

		if(ecu.validNum > 0)
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
			SendToSocketA(SendData ,length+1);
		}else
		{
			sprintf(SendData,"APS110018001600END\n");
			SendToSocketA(SendData ,19);			
		}
	}
	
}


void APP_Response_GetRSDHistoryInfo(char mapping,char *date_time ,char * UID)
{
	int packlength = 0,len_body = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015001701\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength);
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
		SendToSocketA(SendData ,packlength);
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
	
	SendToSocketA(SendData ,packlength);
	

}

void APP_Response_GetShortAddrInfo(char mapping,inverter_info *inverter)
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
	SendToSocketA(SendData ,packlength);

}


void APP_Response_GetECUAPInfo(char mapping,unsigned char connectStatus,char *info)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	if(mapping == 0x00)
	{
		if(0 == connectStatus)
		{
			sprintf(SendData,"APS110019002000%1dEND\n",connectStatus);
			packlength = 20;
		}else
		{
			sprintf(SendData,"APS11%04d002000%1d%sEND\n",(strlen(info) + 19),connectStatus,info);
			packlength = (strlen(info) + 20);
		}
		
	}else
	{
		sprintf(SendData,"APS110015002001\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);

}


//ECU-RS设置WIFI密码
void APP_Response_SetECUAPInfo(unsigned char result)
{
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	sprintf(SendData,"APS1100150021%02d\n",result);
	SendToSocketA(SendData ,16);
}


void APP_Response_GetECUAPList(char mapping,char *list)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);		
	if(mapping == 0x00)
	{
		sprintf(SendData,"APS11%04d002200%sEND\n",(strlen(list) + 18),list);
		packlength = (strlen(list) + 19);		
	}else
	{
		sprintf(SendData,"APS110015002201\n");
		packlength = 16;
	}	
	
	SendToSocketA(SendData ,packlength);
}

void APP_Response_GetFunctionStatusInfo(char mapping)
{
	int packlength = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	

	if(mapping == 0x00)
	{
		sprintf(SendData,"APS110015002300");
		packlength = 15;
		if(ecu.IO_Init_Status == '1')	
		{
			SendData[packlength++] = '2';
		}else
		{
			SendData[packlength++] = '1';
		}
		memset(&SendData[packlength],'0',300);
		packlength += 300;
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
		sprintf(SendData,"APS110015002301\n");
		packlength = 16;
	}		
	SendToSocketA(SendData ,packlength);
}

void APP_Response_ServerInfo(char mapping,ECUServerInfo_t *serverInfo)
{
	int packlength = 0;
	int domain_len = 0;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	
	if(mapping == 0x00)
	{
		Resolve_Server(serverInfo);
		domain_len = strlen(serverInfo->domain);
		if(serverInfo->serverCmdType == SERVER_UPDATE_GET)
		{	//从文件中读取对应的信息
			sprintf(SendData,"APS1100150024%02d00%03d",SERVER_UPDATE_GET,domain_len);
			packlength = 20;
			//域名
			memcpy(&SendData[packlength],serverInfo->domain,domain_len);
			packlength += domain_len;
			//IP
			SendData[packlength++] = serverInfo->IP[0];
			SendData[packlength++] = serverInfo->IP[1];
			SendData[packlength++] = serverInfo->IP[2];
			SendData[packlength++] = serverInfo->IP[3];
			//Port1
			SendData[packlength++] = serverInfo->Port1/256;
			SendData[packlength++] = serverInfo->Port1%256;
			//Port2
			SendData[packlength++] = serverInfo->Port2/256;
			SendData[packlength++] = serverInfo->Port2%256;
			//Port3
			SendData[packlength++] = 0;
			SendData[packlength++] = 0;
			
			
		}else if(serverInfo->serverCmdType == SERVER_CLIENT_GET)
		{
			sprintf(SendData,"APS1100150024%02d00%03d",SERVER_CLIENT_GET,domain_len);
			packlength = 20;
			memcpy(&SendData[packlength],serverInfo->domain,domain_len);
			packlength += domain_len;
			//IP
			SendData[packlength++] = serverInfo->IP[0];
			SendData[packlength++] = serverInfo->IP[1];
			SendData[packlength++] = serverInfo->IP[2];
			SendData[packlength++] = serverInfo->IP[3];
			//Port1
			SendData[packlength++] = serverInfo->Port1/256;
			SendData[packlength++] = serverInfo->Port1%256;
			//Port2
			SendData[packlength++] = serverInfo->Port2/256;
			SendData[packlength++] = serverInfo->Port2%256;
			//Port3
			SendData[packlength++] = 0;
			SendData[packlength++] = 0;
		}else if(serverInfo->serverCmdType == SERVER_CONTROL_GET)
		{
			sprintf(SendData,"APS1100150024%02d00%03d",SERVER_CONTROL_GET,domain_len);
			packlength = 20;
			memcpy(&SendData[packlength],serverInfo->domain,domain_len);
			packlength += domain_len;
			//IP
			SendData[packlength++] = serverInfo->IP[0];
			SendData[packlength++] = serverInfo->IP[1];
			SendData[packlength++] = serverInfo->IP[2];
			SendData[packlength++] = serverInfo->IP[3];
			//Port1
			SendData[packlength++] = serverInfo->Port1/256;
			SendData[packlength++] = serverInfo->Port1%256;
			//Port2
			SendData[packlength++] = serverInfo->Port2/256;
			SendData[packlength++] = serverInfo->Port2%256;
			//Port3
			SendData[packlength++] = 0;
			SendData[packlength++] = 0;
		}else if(serverInfo->serverCmdType == SERVER_UPDATE_SET)
		{
			sprintf(SendData,"APS1100150024%02d00",SERVER_UPDATE_SET);
			Save_Server(serverInfo);
			packlength = 17;
		}else if(serverInfo->serverCmdType == SERVER_CLIENT_SET)
		{
			sprintf(SendData,"APS1100150024%02d00",SERVER_CLIENT_SET);
			Save_Server(serverInfo);
			packlength = 17;
		}else if(serverInfo->serverCmdType == SERVER_CONTROL_SET)
		{
			sprintf(SendData,"APS1100150024%02d00",SERVER_CONTROL_SET);
			Save_Server(serverInfo);
			packlength = 17;
		}else
		{
			return;
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
		sprintf(SendData,"APS1100170024%02d01\n",serverInfo->serverCmdType );
		packlength = 18;
	}		
	SendToSocketA(SendData ,packlength);
}

//ECU-RS设置组网回应
void APP_Response_RegisterThirdInverter(int cmd,unsigned char result)
{
	int packlength = 0,index =0;
	inverter_third_info *curThirdInverter = thirdInverterInfo;
	char uid[54];
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(result == 0x00)
	{
		if(cmd == 1)
		{
			sprintf(SendData,"APS1100170033%02d%02d\n",cmd,result);
			SendToSocketA(SendData ,18);
		}else if(cmd == 2)
		{
			sprintf(SendData,"APS11001500330200");
			packlength = 17;
			for(index=0; (index<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdInverter->inverterid) > 0); index++, curThirdInverter++)
			{
				memset(uid,'\0',54);
				memcpy(uid,curThirdInverter->inverterid,32);
				uid[32] = curThirdInverter->inverter_addr;
				memcpy(&uid[33],curThirdInverter->factory,10);
				memcpy(&uid[43],curThirdInverter->type,10);
				memcpy(&SendData[packlength],uid,53);	
				packlength += 53;
			}
			
			SendData[packlength++] = 'E';
			SendData[packlength++] = 'N';
			SendData[packlength++] = 'D';
			
			SendData[5] = (packlength/1000) + '0';
			SendData[6] = ((packlength/100)%10) + '0';
			SendData[7] = ((packlength/10)%10) + '0';
			SendData[8] = ((packlength)%10) + '0';
			SendData[packlength++] = '\n';
			SendToSocketA(SendData ,packlength);
		}else
		{
			return;
		}
	
	}else
	{
		sprintf(SendData,"APS1100170033%02d%02d\n",cmd,result);
		SendToSocketA(SendData ,18);
	}
}

void APP_Response_GetThirdInverter(unsigned char result)
{
	int packlength = 0,index =0;
	inverter_third_info *curThirdInverter = thirdInverterInfo;
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);
	if(result == 0x00)
	{
		
		if(memcmp(ecu.JsonTime,"00000000000000",14))
		{
			sprintf(SendData,"APS110000003400");
			
			SendData[15] = ecu.thirdCount/256;	
			SendData[16] = ecu.thirdCount%256;

			SendData[17] = (ecu.JsonTime[0] - '0')*16+(ecu.JsonTime[1] - '0');
			SendData[18] = (ecu.JsonTime[2] - '0')*16+(ecu.JsonTime[3] - '0');
			SendData[19] = (ecu.JsonTime[4] - '0')*16+(ecu.JsonTime[5] - '0');
			SendData[20] = (ecu.JsonTime[6] - '0')*16+(ecu.JsonTime[7] - '0');
			SendData[21] = (ecu.JsonTime[8] - '0')*16+(ecu.JsonTime[9] - '0');
			SendData[22] = (ecu.JsonTime[10] - '0')*16+(ecu.JsonTime[11] - '0');
			SendData[23] = (ecu.JsonTime[12] - '0')*16+(ecu.JsonTime[13] - '0');
			packlength = 24;
			for(index=0; (index<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdInverter->inverterid) > 0); index++, curThirdInverter++)
			{
				memcpy(&SendData[packlength],curThirdInverter->inverterid,32);
				packlength += 32;
				SendData[packlength++] = curThirdInverter->third_status.communication_flag;
				memcpy(&SendData[packlength],curThirdInverter->factory,10);
				packlength += 10;
				memcpy(&SendData[packlength],curThirdInverter->type,10);
				packlength += 10;
				//6路PV电压
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[0] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[0] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[1] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[1] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[2] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[2] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[3] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[3] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[4] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[4] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[5] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Voltage[5] *10)%256;
				//6路PV电流
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[0] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[0] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[1] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[1] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[2] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[2] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[3] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[3] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[4] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[4] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[5] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->PV_Current[5] *10)%256;

				//6路 PV功率
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[0] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[0] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[0] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[0] * 100)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[1] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[1] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[1] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[1] * 100)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[2] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[2] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[2] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[2] * 100)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[3] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[3] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[3] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[3] * 100)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[4] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[4] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[4] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[4] * 100)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[5] * 100)/16777216)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[5] * 100)/65535)%256;
				SendData[packlength++] = ((unsigned int)(curThirdInverter->PV_Power[5] * 100)/256)%256;
				SendData[packlength++] = (unsigned int)(curThirdInverter->PV_Power[5] * 100)%256;

				
				//3路AC电压
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[0] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[0] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[1] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[1] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[2] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Voltage[2] *10)%256;
				//3路AC电流
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[0] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[0] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[1] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[1] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[2] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->AC_Current[2] *10)%256;
				//3路AC频率
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[0] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[0] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[1] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[1] *10)%256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[2] *10)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Grid_Frequency[2] *10)%256;
				//温度
				SendData[packlength++] = (unsigned short)((curThirdInverter->Temperature +100)*10)/256;
				SendData[packlength++] = (unsigned short)((curThirdInverter->Temperature +100)*10)%256;

				//有功功率
				SendData[packlength++] =(curThirdInverter->Active_Power/16777216)%256;
				SendData[packlength++] =(curThirdInverter->Active_Power/65536)%256;
				SendData[packlength++] =(curThirdInverter->Active_Power/256)%256;
				SendData[packlength++] =curThirdInverter->Active_Power%256;
				//无功功率
				SendData[packlength++] =(curThirdInverter->Reactive_Power/16777216)%256;
				SendData[packlength++] =(curThirdInverter->Reactive_Power/65536)%256;
				SendData[packlength++] =(curThirdInverter->Reactive_Power/256)%256;
				SendData[packlength++] =curThirdInverter->Reactive_Power%256;
				//功率因素
				SendData[packlength++] = (unsigned short)(curThirdInverter->Power_Factor *1000)/256;
				SendData[packlength++] = (unsigned short)(curThirdInverter->Power_Factor *1000)%256;

				//当前一轮发电量
				SendData[packlength++] =(unsigned int)((curThirdInverter->Current_Energy*10)/16777216)%256;
				SendData[packlength++] =(unsigned int)((curThirdInverter->Current_Energy*10)/65536)%256;
				SendData[packlength++] =(unsigned int)((curThirdInverter->Current_Energy*10)/256)%256;
				SendData[packlength++] =(unsigned int)(curThirdInverter->Current_Energy*10)%256;
				//日发电量
				SendData[packlength++] =((unsigned int)(curThirdInverter->Daily_Energy*10)/16777216)%256;
				SendData[packlength++] =(unsigned int)((curThirdInverter->Daily_Energy*10)/65536)%256;
				SendData[packlength++] =(unsigned int)((curThirdInverter->Daily_Energy*10)/256)%256;
				SendData[packlength++] =(unsigned int)(curThirdInverter->Daily_Energy*10)%256;
				//历史发电量
				SendData[packlength++] =(unsigned int)(curThirdInverter->Life_Energy/16777216)%256;
				SendData[packlength++] =(unsigned int)(curThirdInverter->Life_Energy/65536)%256;
				SendData[packlength++] =(unsigned int)(curThirdInverter->Life_Energy/256)%256;
				SendData[packlength++] =(unsigned int)curThirdInverter->Life_Energy%256;	

				packlength+= 50;
			}
				
			SendData[packlength++] = 'E';
			SendData[packlength++] = 'N';
			SendData[packlength++] = 'D';
				
			SendData[5] = (packlength/1000) + '0';
			SendData[6] = ((packlength/100)%10) + '0';
			SendData[7] = ((packlength/10)%10) + '0';
			SendData[8] = ((packlength)%10) + '0';
			SendData[packlength++] = '\n';
			SendToSocketA(SendData ,packlength);
		}else
		{
			sprintf(SendData,"APS110015003402d\n");
			SendToSocketA(SendData ,16);
		}
	
	}else
	{
		sprintf(SendData,"APS1100150034%02d\n",result);
		SendToSocketA(SendData ,16);
	}
}

void APP_Response_TransmissionZigBeeInfo(char mapping)
{
	memset(SendData,'\0',MAXINVERTERCOUNT*INVERTERLENGTH + 17 + 9);	
	sprintf(SendData,"APS1100150035%02d\n",mapping);
	SendToSocketA(SendData ,16);
}
