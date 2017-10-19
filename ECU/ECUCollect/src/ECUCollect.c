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
#include "serverfile.h"
#include "threadlist.h"
#include <dfs_posix.h>


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


void inverter_Info(inverter_info *curinverter)
{
	printf("\n");

	printf("ID:%02x%02x%02x%02x%02x%02x  ",curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5]);
	printf("LastCollectTime: %s  ",curinverter->LastCollectTime);
	printf("LastCommTime:    %s  ",curinverter->LastCommTime);
	printf("PV1_Energy:%d   ",curinverter->PV1_Energy);
	printf("PV2_Energy:%d   ",curinverter->PV2_Energy);
	printf("Last_PV1_Energy:%d   ",curinverter->Last_PV1_Energy);
	printf("Last_PV2_Energy:%d   ",curinverter->Last_PV2_Energy);
	printf("AveragePower1:%lf   ",curinverter->AveragePower1);
	printf("AveragePower2:%lf  ",curinverter->AveragePower2);
	printf("EnergyPV1:%d  ",curinverter->EnergyPV1);
	printf("EnergyPV2:%d \n",curinverter->EnergyPV2);

}



void Collect_Client_Record(void)
{
	char *client_Data = NULL;
	int length = 0; //当前报文所在位置
	int i = 0;				//轮训变量
	inverter_info *curinverter = inverterInfo;
	int commNum = 0; 	//通讯上的逆变器数量
	char curTime[15] = {'\0'};
	unsigned int CurEnergy = 0;
	unsigned int SysPower = 0;
	apstime(curTime);
	curTime[14] = '\0';
	if(ecu.validNum > 0)
	{
		client_Data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
		memcpy(client_Data,"APS16AAAAA0002AAA1",18);
		memcpy(&client_Data[18],ecu.ECUID12,12);
		memset(&client_Data[30],'0',63);
		memcpy(&client_Data[93],"END",3);
		length = 96;
		//采集一轮数据
		commNum = 0;
		curinverter = inverterInfo;
		rt_enter_critical();
		for(i = 0;i< ecu.validNum; i++)
		{
			//采集每一轮优化器的数据
			//判断数据是否较上一轮有更新，如果更新了，就需要上传，如果没更新就不上传   只有最新一轮通讯打野上一次采集，才会进入
			if(((!memcmp(curinverter->LastCollectTime,"00000000000000",14))&&(memcmp(curinverter->LastCommTime,"00000000000000",14))) || (Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime) > 0))
			//if(1)
			{
				commNum++;		
				curinverter->status.comm_status = 1;

				//内部数据有更新
				//ID	12字节
				memcpy(&client_Data[length],curinverter->uid,12);
				length+=12;
				//输出电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV_Output*100);
				length += 6;
				//输出电流 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PI_Output*100);
				length += 6;
				//输出功率 6字节
				sprintf(&client_Data[length],"%06d",curinverter->Power_Output*100);
				length += 6;
				//输出电量 10字节
				if(curinverter->Last_PV_Output_Energy > curinverter->PV_Output_Energy)
				{
					curinverter->EnergyPV_Output= curinverter->PV_Output_Energy;
					sprintf(&client_Data[length],"%010d",(curinverter->PV_Output_Energy/36));
					length += 10;
				}else
				{
					
					curinverter->EnergyPV_Output= (curinverter->PV_Output_Energy - curinverter->Last_PV_Output_Energy);
					//pv1输入电量(两轮计算差值)
					sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV_Output/36));
					length += 10;	
				}			
				//温度 3字节
				memcpy(&client_Data[length],"100",3);
				length += 3;
				//Optimizer_pv1  1字节
				client_Data[length++]  = '1';
				// PV1输入电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV1*100);
				length += 6;
				// pv1输入电流 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PI*100);
				length += 6;
				//如果当前一轮电量小于上一轮的电量  我们默认为重启过了，电量直接获取，功率为当时的瞬时功率
				if(curinverter->Last_PV1_Energy > curinverter->PV1_Energy)
				{
					curinverter->AveragePower1 = curinverter->Power1;
					sprintf(&client_Data[length],"%06d",(curinverter->Power1*100));
					length += 6;
					curinverter->EnergyPV1 = curinverter->PV1_Energy;
					sprintf(&client_Data[length],"%010d",(curinverter->PV1_Energy/36));
					length += 10;
				}else
				{
					unsigned short Power1;
					//功率=(当前一轮电量-上一轮电量)/时间的差值
					Power1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy)/Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime);
					curinverter->AveragePower1 = Power1;
					//pv1输入功率(计算得到平均功率)
					sprintf(&client_Data[length],"%06d",((unsigned short)curinverter->AveragePower1*100));
					length += 6;
					
					curinverter->EnergyPV1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy);
					//pv1输入电量(两轮计算差值)
					sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV1/36));
					length += 10;	
				}
										
				//Optimizer_pv2  1字节
				client_Data[length++]  = '2';
				// PV2输入电压 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PV2*100);
				length += 6;
				// pv2输入电流 6字节
				sprintf(&client_Data[length],"%06d",curinverter->PI2*100);
				length += 6;
				//如果当前一轮电量小于上一轮的电量  我们默认为重启过了，电量直接获取，功率为当时的瞬时功率
				if(curinverter->Last_PV2_Energy > curinverter->PV2_Energy)
				{
					curinverter->AveragePower2 = curinverter->Power2;
					sprintf(&client_Data[length],"%06d",(curinverter->Power2*100));
					length += 6;
					curinverter->EnergyPV2 = curinverter->PV2_Energy;
					sprintf(&client_Data[length],"%010d",(curinverter->PV2_Energy/36));
					length += 10;
				}else
				{
					unsigned short Power2;
					//功率=(当前一轮电量-上一轮电量)/时间的差值
					Power2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy)/Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime);
					curinverter->AveragePower2 = Power2;
					//pv2输入功率(计算得到平均功率)
					sprintf(&client_Data[length],"%06d",((unsigned short)curinverter->AveragePower2*100));
					length += 6;
					
					curinverter->EnergyPV2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy);
					//pv2输入电量(两轮计算差值)
					sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV2/36));
					length += 10;
				}
					
				//END
				client_Data[length++] = 'E';
				client_Data[length++] = 'N';
				client_Data[length++] = 'D';
				//打印相关信息
				//inverter_Info(curinverter);

#if 0
				if((curinverter->EnergyPV1 > 80000) || (curinverter->EnergyPV2 > 80000))  //如果发电量大于 1 度  表示异常数据   存储
				{
					char *data = NULL;
					data = malloc(4096);
					sprintf(data,"LastCommTime:%s LastCollectTime:%s uid:%s PV1:%d PV2:%d PI:%d PV_output:%d Power1:%d Power2:%d off_time:%d heart_rate:%d pv1_energy:%d pv2_energy:%d mos_close:%d Last_PV1_Energy:%d Last_PV2_Energy:%d",curinverter->LastCommTime,curinverter->LastCollectTime,UID,
							curinverter->PV1,curinverter->PV2,curinverter->PI,curinverter->PV_Output,curinverter->Power1,curinverter->Power2,curinverter->off_times,curinverter->heart_rate,curinverter->PV1_Energy,curinverter->PV2_Energy,curinverter->Mos_CloseNum,curinverter->Last_PV1_Energy,curinverter->Last_PV2_Energy);
					save_dbg(data);
					free(data);
					data = NULL;
				}
#endif 				
				
				memcpy(curinverter->LastCollectTime,curinverter->LastCommTime,15);
				curinverter->LastCollectTime[14] = '\0';
				curinverter->Last_PV1_Energy = curinverter->PV1_Energy;
				curinverter->Last_PV2_Energy = curinverter->PV2_Energy;
				curinverter->Last_PV_Output_Energy = curinverter->PV_Output_Energy;	
			}
			else
			{
				curinverter->status.comm_status = 0;
			}		
			curinverter++;
		}
		
		ecu.count= commNum;
		//计算ECU级别相关数据
		if(commNum > 0)
		{
			char system_power_str[11] = {'\0'};
			char current_energy_str[11] = {'\0'};
			char commNum_str[4] = {'\0'};
			
			curinverter = inverterInfo;
			for(i = 0;i< ecu.validNum; i++)
			{
				//如果该台采集的时候成功
				if(curinverter->status.comm_status == 1)
				{
					CurEnergy = CurEnergy + curinverter->EnergyPV1 + curinverter->EnergyPV2;
					SysPower = SysPower + curinverter->AveragePower1 + curinverter->AveragePower2;
				}
				curinverter++;
			}
			//当前一轮输入功率
			sprintf(system_power_str,"%010d",(long)SysPower*100);
			memcpy(&client_Data[30],system_power_str,10);
			//当前一轮输入电量
			sprintf(current_energy_str,"%010d",(long)CurEnergy/36);
			memcpy(&client_Data[50],current_energy_str,10);
			memcpy(&client_Data[70],curTime,14);
			//最后一次通讯上的次数
			ecu.lastCommNum = commNum;
			sprintf(commNum_str,"%03d",commNum);
			memcpy(&client_Data[84],commNum_str,3);
			
			client_Data[5] = (length/10000)%10 + 0x30;
			client_Data[6] = (length/1000)%10+ 0x30;
			client_Data[7] = (length/100)%10+ 0x30;
			client_Data[8] = (length/10)%10+ 0x30;
			client_Data[9] = (length/1)%10+ 0x30;
			
		}
		client_Data[length++] = '\0';	//存入文件的时候不添加换行符，上传数据的时候再添加换行符
		rt_exit_critical();

		ecu.current_energy = (float)CurEnergy/36000000;		//将发电量转换为千瓦时
		ecu.system_power = SysPower/10;
		ecu.life_energy = ecu.life_energy + ecu.current_energy;
		
		printfloatmsg(ECU_DBG_COLLECT,"ecu.current_energy",ecu.current_energy);
		printfloatmsg(ECU_DBG_COLLECT,"ecu.system_power",ecu.system_power);
		printfloatmsg(ECU_DBG_COLLECT,"ecu.life_energy",ecu.life_energy);
		update_life_energy(ecu.life_energy);								//设置系统历史发电量
		//保存报文数据
		if(commNum > 0)
		{
			
			save_system_power(ecu.system_power,curTime);			//保存系统功率    	按照月存放
			update_daily_energy(ecu.current_energy,curTime);		//保存每日发电量  	按照天存放
			update_monthly_energy(ecu.current_energy,curTime);		//保存每月的发电量	按照月存放
			update_yearly_energy(ecu.current_energy,curTime);  		//保存每年的发电量
			//最多保存两个月的数据
			delete_system_power_2_month_ago(curTime);		//删除两个月前的数据
			save_record(client_Data,curTime);				//保存需要发送给服务器的报文
			//print2msg(ECU_DBG_COLLECT,"client Data:",client_Data);

			//保存数据到文件中 该数据为最后一次的发电量
			save_last_collect_info();
			//保存07命令相关数据到文件中  
			//save_collect_info(curTime);	
			//最多保存2天的数据
			//delete_collect_info_2_day_ago(curTime);//删除两天前的数据
			
		}
		
		free(client_Data);
		client_Data = NULL;
	}
}

void Collect_Control_Record(void)
{
	char *control_Data = NULL;
	int length = 0; //当前报文所在位置
	int i = 0;				//轮训变量
	inverter_info *curinverter = inverterInfo;
	int commNum = 0; 	//通讯上的逆变器数量
	char curTime[15] = {'\0'};
	apstime(curTime);
	
	if(ecu.validNum > 0)
	{
		control_Data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
		//头信息
		memcpy(control_Data,"APS13AAAAAA158AAA1",18);
		//ECU头信息
		memcpy(&control_Data[18],ecu.ECUID12,12);
		memset(&control_Data[30],'0',18);
		memcpy(&control_Data[48],"END",3);
		length = 51;
		//优化器信息
		commNum = 0;
		curinverter = inverterInfo;
		for(i = 0;i< ecu.validNum; i++)
		{
			if(curinverter->status.comm_status == 1)
			{
				commNum++;		

				//内部数据有更新
				//ID	12字节
				memcpy(&control_Data[length],curinverter->uid,12);
				length += 12;
				sprintf(&control_Data[length],"%05d",curinverter->heart_rate);
				length += 5;
				sprintf(&control_Data[length],"%05d",curinverter->off_times);
				length += 5;
				sprintf(&control_Data[length],"%03d",curinverter->restartNum);
				length += 3;
				sprintf(&control_Data[length],"%03d",curinverter->RSSI);
				length += 3;
				memcpy(&control_Data[length],"0000000000",10);
				length += 10;

				control_Data[length++] = 'E';
				control_Data[length++] = 'N';
				control_Data[length++] = 'D';
	
			}	
			curinverter++;
		}
		
		
		//计算ECU级别相关数据
		if(commNum > 0)
		{
			char commNum_str[5] = {'\0'};


			memcpy(&control_Data[34],curTime,14);
			//最后一次通讯上的次数
			ecu.lastCommNum = commNum;
			sprintf(commNum_str,"%04d",commNum);
			memcpy(&control_Data[30],commNum_str,4);
			
			control_Data[5] = (length/10000)%10 + 0x30;
			control_Data[6] = (length/1000)%10+ 0x30;
			control_Data[7] = (length/100)%10+ 0x30;
			control_Data[8] = (length/10)%10+ 0x30;
			control_Data[9] = (length/1)%10+ 0x30;
			
		}
		control_Data[length++] = '\0';	//存入文件的时候不添加换行符，上传数据的时候再添加换行符
		
		//保存报文数据
		if(commNum > 0)
		{		
			save_control_record(control_Data,curTime);
			//print2msg(ECU_DBG_COLLECT,"client Data:",control_Data);
			
		}
		
		free(control_Data);
		control_Data = NULL;
	}
	
}


//该线程主要用于相关数据的采集工作
void ECUCollect_thread_entry(void* parameter)
{

	int CollectClientThistime=0, CollectClientDurabletime=65535, CollectClientReportinterval=300;			//采集数据相关时间参数
	int CollectControlThistime=0, CollectControlDurabletime=65535, CollectControlReportinterval=900;	//采集远程控制数据时间参数
	
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_COLLECT);
	while(1)
	{
		
		if(compareTime(CollectClientDurabletime ,CollectClientThistime,CollectClientReportinterval))
		//if(compareTime(CollectClientDurabletime ,CollectClientThistime,30))
		{
			optimizeFileSystem();
			printmsg(ECU_DBG_COLLECT,"Collect DATA Start");
			//5分钟采集相关的发电量数据
			CollectClientThistime = acquire_time();
			//采集实时数据
			Collect_Client_Record();
			printmsg(ECU_DBG_COLLECT,"Collect DATA End");

		}
		
		if(compareTime(CollectControlDurabletime ,CollectControlThistime,CollectControlReportinterval))
		{	
			optimizeFileSystem();
			//采集心跳相关远程控制数据
			printmsg(ECU_DBG_COLLECT,"Collect Control DATA  Start");
			CollectControlThistime = acquire_time();
			//采集远程控制数据
			Collect_Control_Record();
			printmsg(ECU_DBG_COLLECT,"Collect Control DATA  End");

		}
		

		rt_thread_delay(RT_TICK_PER_SECOND);
		CollectClientDurabletime = acquire_time();		
		CollectControlDurabletime = acquire_time();			
	}
}
