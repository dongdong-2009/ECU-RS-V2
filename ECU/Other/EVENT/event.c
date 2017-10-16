/*****************************************************************************/
/* File      : event.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-09 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "event.h"
#include "string.h"
#include "usart5.h"
#include "appcomm.h"
#include "SEGGER_RTT.h"
#include "inverter.h"
#include "rthw.h"
#include "led.h"
#include "serverfile.h"
#include "version.h"
#include "debug.h"
#include "AppFunction.h"
#include "serverfile.h"
#include "zigbee.h"
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define PERIOD_NUM			3600

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
unsigned short comm_failed_Num = 0;
unsigned short pre_heart_rate;
extern rt_mutex_t wifi_uart_lock;
int Data_Len = 0,Command_Id = 0;
int ResolveFlag = 0;


//测试枚举
typedef enum
{ 
    HARDTEST_TEST_ALL    	= 0,		//接收数据头
    HARDTEST_TEST_EEPROM  = 1,	//接收数据长度   其中数据部分的长度为接收到长度减去12个字节
    HARDTEST_TEST_WIFI  	= 2,	//接收数据部分数据
    HARDTEST_TEST_433  		= 3
} eHardWareID;// receive state machin

enum CommandID{
	P00, P01, P02, P03, P04, P05, P06, P07, P08, P09, //0-9
	P10, P11, P12, P13, P14, P15, P16, P17, P18, P19, //10-19
	P20, P21, P22, P23, P24, P25, P26, P27, P28, P29, //20-29
	P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, //30-39
	P40, P41, P42, P43, P44, P45, P46, P47, P48, P49, //40-49
	P50, P51, P52, P53, P54, P55, P56, P57, P58, P59, //50-59
};

void (*pfun_Phone[100])(unsigned char * id,int DataLen,const char *recvbuffer);

void add_APP_functions(void)
{
	pfun_Phone[P01] = App_GetBaseInfo; 				//获取基本信息请求
	pfun_Phone[P02] = App_GetSystemInfo; 				//获取系统信息
	pfun_Phone[P03] = App_SetNetwork; 				//设置组网
	pfun_Phone[P04] = App_SetChannel; 				//设置信道
	pfun_Phone[P05] = App_SetWIFIPasswd; 			//设置WIFI密码
	pfun_Phone[P06] = App_SetIOInitStatus; 			//设置IO初始状态
	pfun_Phone[P07] = APP_GetRSDHistoryInfo; 		//功率电流电压曲线
	pfun_Phone[P08] = App_GetGenerationCurve; 				//发电量曲线请求
	pfun_Phone[P09] = App_SetWiredNetwork; 	//有线网络设置
	pfun_Phone[P10] = App_GetWiredNetwork; 		//获取有线网络设置
	pfun_Phone[P11] = App_GetFlashSize; 			//获取Flash剩余空间
	pfun_Phone[P12] = App_GetPowerCurve; 			//获取Flash剩余空间


}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//WIFI事件处理
void process_WIFI(unsigned char * ID)
{
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvSocketAData,&Data_Len,&Command_Id);
	printf("%d %d %s\n",ResolveFlag,Command_Id,WIFI_RecvSocketAData);
	if(ResolveFlag == 0)
	{
		add_APP_functions();
		//函数指针不为空，则运行对应的函数
		if(pfun_Phone[Command_Id%100])
		{
			printf("pfun_Phone ID:%d\n",Command_Id);
			(*pfun_Phone[Command_Id%100])(ID,Data_Len,(char *)WIFI_RecvSocketAData);
		}
		
	}

}

//硬件测试   返回值是测试的错误码
int HardwareTest(char testItem)
{
	switch(testItem)
	{
		case HARDTEST_TEST_ALL:
			//测试WIFI模块
			if(0 != WIFI_Test())
			{
				printf("WIFI abnormal\n");
				return 2;
			}
			
			//测试433模块

			
			break;
		
		case HARDTEST_TEST_EEPROM:
			//测试EEPROM读写
			break;
		
		case HARDTEST_TEST_WIFI:
			//测试WIFI模块
			if(0 != WIFI_Test())
			{
				return 2;
			}
			break;
		
		case HARDTEST_TEST_433:
			//测试433模块

			break;
				
	}
	return 0;
}


void process_WIFIEvent(void)
{
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	//检测WIFI事件
	WIFI_GetEvent();
	rt_mutex_release(wifi_uart_lock);
	//判断是否有WIFI接收事件
	if(WIFI_Recv_SocketA_Event == 1)
	{
		SEGGER_RTT_printf(0,"WIFI_Recv_Event start\n");
		process_WIFI(ID_A);
		WIFI_Recv_SocketA_Event = 0;
		SEGGER_RTT_printf(0,"WIFI_Recv_Event end\n");
	}

}


//心跳事件处理
void process_HeartBeatEvent(void)
{
	int ret = 0;
	
	if(	ecu.validNum >0	)
	{
		if(ecu.curSequence >= ecu.validNum)		//当轮训的序号大于最后一台时，更换到第0台
		{
			ecu.curSequence = 0;
		}
		pre_heart_rate = inverterInfo[ecu.curSequence].heart_rate;
		//发送心跳命令
		ret = zb_query_heart_data(&inverterInfo[ecu.curSequence]);
		if(ret == -1)	//发送心跳包失败
		{
			inverterInfo[ecu.curSequence].status.heart_Failed_times++;
			if(inverterInfo[ecu.curSequence].status.heart_Failed_times >= 3)
			{
				inverterInfo[ecu.curSequence].status.heart_Failed_times = 3;		
				inverterInfo[ecu.curSequence].status.comm_failed3_status = 0;
			}
				
			//通信失败，失败次数++
			comm_failed_Num ++;
		}else	//发送心跳包成功
		{
			//通信失败，失败次数++
			comm_failed_Num  = 0;
			inverterInfo[ecu.curSequence].status.heart_Failed_times = 0;
		
			//如果当前一轮心跳小于上一轮心跳,表示重启
			if(inverterInfo[ecu.curSequence].heart_rate < pre_heart_rate)
			{
				//当前一轮重启次数+1
				if(inverterInfo[ecu.curSequence].restartNum < 255)
					inverterInfo[ecu.curSequence].restartNum++;
			}
			
		}
		
		ecu.curSequence++;
		
		
		//连续通讯不上1小时   表示关机状态
		if(comm_failed_Num >= PERIOD_NUM)
		{
			//SEGGER_RTT_printf(0, "comm_failed_Num:%d   \n",comm_failed_Num);
			for(ecu.curSequence = 0;ecu.curSequence < ecu.validNum;ecu.curSequence++)
			{
				inverterInfo[ecu.curSequence].restartNum = 0;
			}
			comm_failed_Num = 0;
			ecu.curSequence = 0;
		}
	}

}

//按键初始化密码事件处理
void process_KEYEvent(void)
{
	int ret =0,i = 0;
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event Start\n");
		rt_hw_ms_delay(5000);


	for(i = 0;i<3;i++)
	{
		
		ret = WIFI_Factory(ecu.ECUID12);
		if(ret == 0) break;
	}
	
	if(ret == 0) 	//写入WIFI密码
		Write_WIFI_PW("88888888",8);	//WIFI密码	
	
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event End\n");
}

//无线复位处理
int process_WIFI_RST(void)
{
	int ret =1,i = 0;
	SEGGER_RTT_printf(0, "process_WIFI_RST Start\n");
	for(i = 0;i<3;i++)
	{
		ret = WIFI_SoftReset();
		if(ret == 0) break;
	}
	SEGGER_RTT_printf(0, "process_WIFI_RST End\n");
	return ret;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
int setECUID(char *ECUID)
{
	char ret =0;
	if(strlen(ECUID) != 12)
	{
		printf("ECU ID Length misMatching\n");
		return -1;
	}

	ret = Write_ECUID(ECUID);													  		//ECU ID
	//设置WIFI密码
	if(ret != 0) 	
	{
		printf("ECU ID Write EEPROM Failed\n");
		return -1;
	}
	echo("/config/channel.con","0x10");
	ecu.IO_Init_Status= '1';
	Write_IO_INIT_STATU(&ecu.IO_Init_Status);
	//设置WIFI密码
	rt_hw_ms_delay(5000);
	ret = WIFI_Factory(ECUID);
	//写入WIFI密码
	Write_WIFI_PW("88888888",8);	//WIFI密码		
	init_ecu();
	return 0;
	
}
FINSH_FUNCTION_EXPORT(setECUID, eg:set ECU ID("247000000001"));


int ReadECUID(void)
{
	char USART1_ECUID12[13] = {'\0'};
	char USART1_ECUID6[7] = {'\0'};
	
	Read_ECUID(USART1_ECUID6);
	transformECUID(USART1_ECUID6,USART1_ECUID12);		//转换ECU ID
	USART1_ECUID12[12] = '\0';
	
	if(USART1_ECUID6[0] == 0xff)
	{
		printf("undesirable ID\n");
		return -1;
	}else
	{
		printf("ID : %s\n",USART1_ECUID12);
		return 0;
	}
}
FINSH_FUNCTION_EXPORT(ReadECUID, eg:Read ECU ID);

int Test(void)
{
	HardwareTest(0);
	return 0;
}
FINSH_FUNCTION_EXPORT(Test, eg:Test Hardware);

int FactoryStatus(void)
{
	//删除所有的UID文件
	init_inverter(inverterInfo);
	return 0;
}
FINSH_FUNCTION_EXPORT(FactoryStatus, eg:recover Factory Status);

#endif



#ifdef RT_USING_FINSH
#include <finsh.h>
void baseInfo(void)
{
	int i = 0 , type = inverterInfo[0].status.device_Type;
	for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)
	{
		if(type != inverterInfo[i].status.device_Type)		//判断是否相等
		{
			type = 2;
			break;
		}
	}
	printf("\n");
	printf("************************************************************\n");
	printf("ECU ID : %s\n",ecu.ECUID12);
	printf("ECU Version :%s\n",VERSION_ECU_RS);
	printf("ECU Channel :%02x\n",ecu.channel);
	printf("ECU RSSI :%d\n",ecu.Signal_Level);
	printf("RSD Type :%d\n",type);
	printf("ECU software Version : %s_%s_%s\n",ECU_VERSION,MAJORVERSION,MINORVERSION);
	printf("************************************************************\n");
}
FINSH_FUNCTION_EXPORT(baseInfo, eg:baseInfo());

void systemInfo(void)
{
	int i = 0;
	inverter_info *curinverter = inverterInfo;
	printf("\n");
	printf("*****ID*****Type*OnOff*Function*PV1Flag*PV2Flag*HeartTime*Timeout*CloseTime***PV1******PV2******PI******P1*****P2******PV1EN*******PV2EN***RSSI****MOSCL*****EnergyPV1******EnergyPV2\n");
	for(i=0; (i<MAXINVERTERCOUNT)&&(i < ecu.validNum); i++)	
	{
		printf("%02x%02x%02x%02x%02x%02x ",curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5]);
		printf("%1d ",curinverter->status.device_Type);
		printf("    %1d ",curinverter->status.comm_failed3_status);
		printf("     %1d ",curinverter->status.function_status);
		printf("      %1d ",curinverter->status.pv1_low_voltage_pritection);
		printf("      %1d ",curinverter->status.pv2_low_voltage_pritection);
		printf("      %5d ",curinverter->heart_rate);
		printf("  %5d ",curinverter->off_times);
		printf(" %5d ",curinverter->restartNum);
		printf("     %5u ",curinverter->PV1);
		printf("   %5u ",curinverter->PV2);
		printf("  %5u ",curinverter->PI);
		printf("  %5u ",curinverter->Power1);
		printf(" %5u ",curinverter->Power2);
		printf("%10u ",curinverter->PV1_Energy);
		printf(" %10u ",curinverter->PV2_Energy);
		printf(" %5d ",curinverter->RSSI);
		printf(" %10u ",curinverter->EnergyPV1);
		printf(" %10u ",curinverter->EnergyPV2);
		printf("     %3d \n",curinverter->Mos_CloseNum);
		curinverter++;
	}
	printf("*****************************************************************************************************************************************\n");
}
FINSH_FUNCTION_EXPORT(systemInfo, eg:systemInfo());

#endif

