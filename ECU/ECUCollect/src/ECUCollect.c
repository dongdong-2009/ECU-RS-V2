#include "ECUCollect.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rtc.h"


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


void Collect_Client_Record(void)
{
	char *client_Data = NULL;
	int length = 0; //当前报文所在位置
	int i = 0;				//轮训变量
	inverter_info *curinverter = inverterInfo;
	int commNum = 0; 	//通讯上的逆变器数量
	
	if(ecu.validNum > 0)
	{
		client_Data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT);
		memcpy(client_Data,"APS16AAAAA0002AAA1",18);
		memcpy(&client_Data[18],ecu.ECUID12,12);
		memset(&client_Data[30],'0',63);
		memcpy(&client_Data[93],"END",3);
		length = 96;
		//采集一轮数据
		commNum = 0;
		curinverter = inverterInfo;
		for(i = 0;i< ecu.validNum; i++)
		{
			//采集每一轮优化器的数据
			//判断数据是否较上一轮有更新，如果更新了，就需要上传，如果没更新就不上传
			if(Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime) > 0)
			{
				commNum++;		
				//获取当前时间
				apstime(curinverter->CurCollectTime);
				//内部数据有更新
				//ID	12字节
				client_Data[length++] = (curinverter->uid[0]/16) + '0';
				client_Data[length++] = (curinverter->uid[0]%16) + '0';
				client_Data[length++] = (curinverter->uid[1]/16) + '0';
				client_Data[length++] = (curinverter->uid[1]%16) + '0';
				client_Data[length++] = (curinverter->uid[2]/16) + '0';
				client_Data[length++] = (curinverter->uid[2]%16) + '0';
				client_Data[length++] = (curinverter->uid[3]/16) + '0';
				client_Data[length++] = (curinverter->uid[3]%16) + '0';
				client_Data[length++] = (curinverter->uid[4]/16) + '0';
				client_Data[length++] = (curinverter->uid[4]%16) + '0';
				client_Data[length++] = (curinverter->uid[5]/16) + '0';
				client_Data[length++] = (curinverter->uid[5]%16) + '0';
				//输出电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV_Output);
				length += 6;
				//输出电流 6字节
				memcpy(&client_Data[length],"000000",6);
				length += 6;
				//输出功率 6字节
				memcpy(&client_Data[length],"000000",6);
				length += 6;
				//输出功率 10字节
				memcpy(&client_Data[length],"0000000000",10);
				length += 10;
				//温度 3字节
				memcpy(&client_Data[length],"000",3);
				length += 3;
				//Optimizer_pv1  1字节
				client_Data[length++]  = 1;
				// PV1输入电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV1);
				length += 6;
				// pv1输入电流 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PI);
				length += 6;
				//如果当前一轮电量小于上一轮的电量  我们默认为重启过了，电量直接获取，功率为当时的瞬时功率
				if(curinverter->Last_PV1_Energy > curinverter->PV1_Energy)
				{
					sprintf(&client_Data[length],"%06d",(curinverter->Power1*100));
					length += 6;
					sprintf(&client_Data[length],"%010d",(curinverter->PV1_Energy*1000000));
					length += 10;
				}else
				{
					unsigned short Power1;
					//功率=(当前一轮电量-上一轮电量)/时间的差值
					Power1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy)/Time_difference(curinverter->CurCollectTime,curinverter->LastCollectTime);
					//pv1输入功率(计算得到平均功率)
					
					//pv1输入电量(两轮计算差值)
				}
										
				//Optimizer_pv2  1字节
				client_Data[length++]  = 2;
				// PV2输入电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV2);
				length += 6;
				// pv2输入电流 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PI);
				length += 6;
				//如果当前一轮电量小于上一轮的电量  我们默认为重启过了，电量直接获取，功率为当时的瞬时功率
				if(curinverter->Last_PV2_Energy > curinverter->PV2_Energy)
				{
					sprintf(&client_Data[length],"%06d",(curinverter->Power2*100));
					length += 6;
					sprintf(&client_Data[length],"%010d",(curinverter->PV2_Energy*1000000));
					length += 10;
				}else
				{
					unsigned short Power2;
					//功率=(当前一轮电量-上一轮电量)/时间的差值
					Power2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy)/Time_difference(curinverter->CurCollectTime,curinverter->LastCollectTime);
					//pv2输入功率(计算得到平均功率)
				
					//pv2输入电量(两轮计算差值)
				}
					
				//END
				client_Data[length++] = 'E';
				client_Data[length++] = 'N';
				client_Data[length++] = 'D';
				
				
						
				memcpy(curinverter->LastCollectTime,curinverter->CurCollectTime,15);
				curinverter->Last_PV1_Energy = curinverter->PV1_Energy;
				curinverter->Last_PV2_Energy = curinverter->PV2_Energy;
				//保存数据到文件中
				
						
			}
					
					
			curinverter++;
		}
		client_Data[length++] = '\n';
		//计算ECU级别相关数据
		
		//保存数据
		printf("client Data:%s\n",client_Data);
		free(client_Data);
		client_Data = NULL;
	}
}



//该线程主要用于相关数据的采集工作
void ECUCollect_thread_entry(void* parameter)
{

	int CollectClientThistime=0, CollectClientDurabletime=65535, CollectClientReportinterval=300;			//采集数据相关时间参数
	int CollectControlThistime=0, CollectControlDurabletime=65535, CollectControlReportinterval=900;	//采集远程控制数据时间参数
	
	while(1)
	{
		if(compareTime(CollectClientDurabletime ,CollectClientThistime,CollectClientReportinterval))
		{
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     DATA  Start-------------------------");
			//5分钟采集相关的发电量数据
			CollectClientThistime = acquire_time();
		
			
			
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     DATA  End-------------------------");

		}
		
		
		
		if(compareTime(CollectControlDurabletime ,CollectControlThistime,CollectControlReportinterval))
		{	
			//采集心跳相关远程控制数据
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     Control DATA  Start-------------------------");

			CollectControlThistime = acquire_time();
			
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     Control DATA  End-------------------------");

		}

		rt_thread_delay(RT_TICK_PER_SECOND);
		CollectClientDurabletime = acquire_time();		
		CollectControlDurabletime = acquire_time();			
	}
}
