/*****************************************************************************/
/* File      : third_inverter_HW.c                                           */
/*  History:                                                                 */
/*  Date       * Author          * Changes                                   */
/*  2018-05-8 * miaoweijie  * Creation of the file                           */
/*             *                 *                                           */
/*****************************************************************************/

/*  Include Files */
#include "third_inverter_HW.h"
#include "third_inverter.h"
#include "modbus.h"
#include "string.h"
#include "stdio.h"
#include "usart485.h"

/*  Definitions */
#define HW_DEVICEINFO_ADDRESS	32262
#define HW_DEVICEINFO_LENGTH	100 

extern ecu_info ecu;


/*****************************************************************************/
/* Function Name :  ResolveData_HW_DeviceInfo                                */
/* Function Description: Get SA Inverter Data                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
static void ResolveData_HW_DeviceInfo(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    float Daily_Energy_last = curThirdinverter->Daily_Energy;
	unsigned int uiLoop = 0;

	//hw��·,�ӵ�ַ32262��ʼ
	for(uiLoop = 0; uiLoop < MAX_PV_NUM; uiLoop++)
	{
	    curThirdinverter->PV_Voltage[uiLoop] = (float)pWords[uiLoop * 2] / 10;	       //ֱ����ѹ
	    curThirdinverter->PV_Current[uiLoop] = (float)pWords[uiLoop * 2 + 1]  / 10;	   //ֱ������
	    curThirdinverter->PV_Power[uiLoop]   = curThirdinverter->PV_Current[uiLoop] *  //ֱ������
			                                   curThirdinverter->PV_Voltage[uiLoop];
	}

	for(uiLoop = 0; uiLoop < MAX_AC_NUM; uiLoop++)
	{
		curThirdinverter->AC_Voltage[uiLoop] = (float)pWords[uiLoop + 15] / 10;	       //���ѹ
	    curThirdinverter->AC_Current[uiLoop] = (float)pWords[uiLoop + 18] / 10;        //�����
	}

	curThirdinverter->Grid_Frequency[0] = (float)pWords[21] / 100;			           //����Ƶ��
	curThirdinverter->Grid_Frequency[1] = curThirdinverter->Grid_Frequency[2]  = 0;
    curThirdinverter->Power_Factor      = (float)((signed short)pWords[22]) / 1000;    //�������� 
    curThirdinverter->Temperature       = (float)((signed short)pWords[24]) / 10;	   //�¶�
    curThirdinverter->Active_Power      = pWords[28] * 65536 + pWords[29];             //�й�����
    curThirdinverter->Reactive_Power    = pWords[30] * 65536 + pWords[31];             //�޹�����
    curThirdinverter->Daily_Energy  = (float)(pWords[38] *65536 + pWords[39]) / 100;   //���췢����
    curThirdinverter->Life_Energy   = (float)(pWords[44] *65536 + pWords[45]) / 100;   //��ʷ������

    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//��ǰһ�ַ�����������һ�ֵķ�����
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy - Daily_Energy_last;
    }
    else
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy;
    }

    return;
}


/*****************************************************************************/
/* Function Name : GetData_HW_DeviceInfo                                     */
/* Function Description: Get DeviceInfo Data1                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
static int GetData_HW_DeviceInfo(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//��ȡ���ֽ�
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_OutWords(curThirdinverter->inverter_addr,
                          HW_DEVICEINFO_ADDRESS, HW_DEVICEINFO_LENGTH,
                          &nbReadWords,pWords);
    if(0 == ret)
    {
        //�豸���Խ���
        ResolveData_HW_DeviceInfo(curThirdinverter, pWords);
        return 0;
    }else
    {
        return -1;
    }
}

/*****************************************************************************/
/* Function Name :  GetData_ThirdInverter_HW                                 */
/* Function Description: Get HW Inverter Data                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
int GetData_ThirdInverter_HW(inverter_third_info *curThirdinverter)
{
    int ret = 0;
    //��ȡ�豸����1
    
    usart485_init(get_ThirdBaudRate(curThirdinverter->cBaudrate));
    ret = GetData_HW_DeviceInfo(curThirdinverter);
    if(0 == ret)
    {
        //�ж�addrflag�󶨱�־�Ƿ�仯
        curThirdinverter->third_status.communication_flag = 1;
        if(curThirdinverter->third_status.inverter_addr_flag == 0)
        {
            curThirdinverter->third_status.inverter_addr_flag = 1;
            ecu.ThirdIDUpdateFlag = 1;
        }

        return 0;
    }
    else
    {
        curThirdinverter->third_status.communication_flag = 0;
        return -1;
    }

}


