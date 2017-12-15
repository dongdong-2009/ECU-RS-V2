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

extern rt_mutex_t wifi_uart_lock;
int Data_Len = 0,Command_Id = 0;
int ResolveFlag = 0;


//����ö��
typedef enum
{ 
    HARDTEST_TEST_ALL    	= 0,		//��������ͷ
    HARDTEST_TEST_EEPROM  = 1,	//�������ݳ���   �������ݲ��ֵĳ���Ϊ���յ����ȼ�ȥ12���ֽ�
    HARDTEST_TEST_WIFI  	= 2,	//�������ݲ�������
    HARDTEST_TEST_433  		= 3
} eHardWareID;// receive state machin

enum CommandID{
	P0000, P0001, P0002, P0003, P0004, P0005, P0006, P0007, P0008, P0009, //0-9
	P0010, P0011, P0012, P0013, P0014, P0015, P0016, P0017, P0018, P0019, //10-19
	P0020, P0021, P0022, P0023, P0024, P0025, P0026, P0027, P0028, P0029, //20-29
	P0030, P0031, P0032, P0033, P0034, P0035, P0036, P0037, P0038, P0039, //30-39
	P0040, P0041, P0042, P0043, P0044, P0045, P0046, P0047, P0048, P0049, //40-49
	P0050, P0051, P0052, P0053, P0054, P0055, P0056, P0057, P0058, P0059, //50-59
};

void (*pfun_Phone[100])(unsigned char * id,int DataLen,const char *recvbuffer);

void add_APP_functions(void)
{
	pfun_Phone[P0001] = App_GetBaseInfo; 				//��ȡ������Ϣ����
	pfun_Phone[P0002] = App_GetSystemInfo; 				//��ȡϵͳ��Ϣ
	pfun_Phone[P0003] = App_GetPowerCurve; 			//��ȡ��������
	pfun_Phone[P0004] = App_GetGenerationCurve; 				//��������������
	pfun_Phone[P0005] = App_SetNetwork; 				//��������
	pfun_Phone[P0006] = App_SetTime; 			//ECUʱ������
	pfun_Phone[P0007] = App_SetWiredNetwork; 	//������������
	pfun_Phone[P0008] = App_GetECUHardwareStatus; 	//�鿴��ǰECUӲ��״̬
	pfun_Phone[P0010] = App_SetWIFIPasswd; 			//����WIFI����
	pfun_Phone[P0011] = App_GetIDInfo; 			//��ȡID��Ϣ
	pfun_Phone[P0012] = App_GetTime; 			//��ȡʱ��
	pfun_Phone[P0013] = App_GetFlashSize; 			//��ȡFlashʣ��ռ�
	pfun_Phone[P0014] = App_GetWiredNetwork; 		//��ȡ������������
	pfun_Phone[P0015] = App_SetChannel; 				//�����ŵ�
	pfun_Phone[P0016] = App_SetIOInitStatus; 			//����IO��ʼ״̬
	pfun_Phone[P0017] = APP_GetRSDHistoryInfo; 		//���ʵ�����ѹ����
	pfun_Phone[P0018] = APP_GetShortAddrInfo;		//���ʵ�����ѹ����



}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//WIFI�¼�����
void process_WIFI(unsigned char * ID)
{
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvSocketAData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		add_APP_functions();
		//����ָ�벻Ϊ�գ������ж�Ӧ�ĺ���
		if(pfun_Phone[Command_Id%100])
		{
			(*pfun_Phone[Command_Id%100])(ID,Data_Len,(char *)WIFI_RecvSocketAData);
		}
		
	}

}

//Ӳ������   ����ֵ�ǲ��ԵĴ�����
int HardwareTest(char testItem)
{
	switch(testItem)
	{
		case HARDTEST_TEST_ALL:
			//����WIFIģ��
			if(0 != WIFI_Test())
			{
				printf("WIFI abnormal\n");
				return 2;
			}
			
			//����433ģ��

			
			break;
		
		case HARDTEST_TEST_EEPROM:
			//����EEPROM��д
			break;
		
		case HARDTEST_TEST_WIFI:
			//����WIFIģ��
			if(0 != WIFI_Test())
			{
				return 2;
			}
			break;
		
		case HARDTEST_TEST_433:
			//����433ģ��

			break;
				
	}
	return 0;
}


void process_WIFIEvent(void)
{
	rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
	//���WIFI�¼�
	WIFI_GetEvent();
	rt_mutex_release(wifi_uart_lock);
	//�ж��Ƿ���WIFI�����¼�
	if(WIFI_Recv_SocketA_Event == 1)
	{
		SEGGER_RTT_printf(0,"WIFI_Recv_Event start\n");
		process_WIFI(ID_A);
		WIFI_Recv_SocketA_Event = 0;
		SEGGER_RTT_printf(0,"WIFI_Recv_Event end\n");
	}

}

//������ʼ�������¼�����
void process_KEYEvent(void)
{
	int ret =0,i = 0;
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event Start\n");

	for(i = 0;i<3;i++)
	{
		
		ret = WIFI_Factory_Passwd();
		if(ret == 0) break;
	}
	
	if(ret == 0) 	//д��WIFI����
	{
		key_init();
		Write_WIFI_PW("88888888",8);	//WIFI����	
	}
		
	
	SEGGER_RTT_printf(0, "KEY_FormatWIFI_Event End\n");
}

//���߸�λ����
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

int setECUID(char *ECUID)
{
	char ret =0;
	if(strlen(ECUID) != 12)
	{
		printf("ECU ID Length misMatching\n");
		return -1;
	}

	ret = Write_ECUID(ECUID);													  		//ECU ID
	//����WIFI����
	if(ret != 0) 	
	{
		printf("ECU ID Write EEPROM Failed\n");
		return -1;
	}
	echo("/config/channel.con","0x17");
	ecu.IO_Init_Status= '1';
	Write_IO_INIT_STATU(&ecu.IO_Init_Status);
	//����WIFI����
	ret = WIFI_Factory(ECUID);
	//д��WIFI����
	Write_WIFI_PW("88888888",8);	//WIFI����		
	init_ecu();
	return 0;
	
}


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(setECUID, eg:set ECU ID("247000000001"));


int ReadECUID(void)
{
	char USART1_ECUID12[13] = {'\0'};
	char USART1_ECUID6[7] = {'\0'};
	
	Read_ECUID(USART1_ECUID6);
	transformECUID(USART1_ECUID6,USART1_ECUID12);		//ת��ECU ID
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
	//ɾ�����е�UID�ļ�
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
		if(type != inverterInfo[i].status.device_Type)		//�ж��Ƿ����
		{
			type = 2;
			break;
		}
	}
	printf("\n");
	printf("************************************************************\n");
	printf("ECU ID : %s\n",ecu.ECUID12);
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
		printf("%s ",curinverter->uid);
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

