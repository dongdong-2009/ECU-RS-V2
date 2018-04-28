/*****************************************************************************/
/* File      : third_inverter_SA.c                                           */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-04-26 * miaoweijie  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*  Include Files */
#include "third_inverter_SA.h"
#include "modbus.h"
#include "string.h"
#include "stdio.h"

/*  Definitions */
#define SA_DEVICEINFO_ADDRESS	256    //0100H
#define SA_DEVICEINFO_LENGTH	56     //0137-0100+1

extern ecu_info ecu;


/*****************************************************************************/
/* Function Name :  ResolveData_SG_DeviceInfo1                               */
/* Function Description: Get SA Inverter Data                                */
/* date :2018-04-26                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
static int ResolveData_SA_DeviceInfo(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    float Daily_Energy_last = curThirdinverter->Daily_Energy;

    //type
    //�������·

    curThirdinverter->PV_Voltage[0] = (float)pWords[7]	/ 10;			//ֱ����ѹ1
    curThirdinverter->PV_Current[0] = (float)pWords[8]  / 100;			//ֱ������1
    curThirdinverter->PV_Voltage[1] = (float)pWords[10]	/ 10;			//ֱ����ѹ2
    curThirdinverter->PV_Current[1] = (float)pWords[11] / 100;			//ֱ������2
    curThirdinverter->PV_Voltage[2] = (float)pWords[13]	/ 10;			//ֱ����ѹ3
    curThirdinverter->PV_Current[2] = (float)pWords[14] / 100;			//ֱ������3
    curThirdinverter->Temperature   = (float)((signed short)pWords[17]) / 10;	//�¶�

    curThirdinverter->Active_Power  = pWords[19];                       //�й�����
    curThirdinverter->Reactive_Power = pWords[20];                      //�޹�����
    curThirdinverter->Power_Factor  = (float)((signed short)pWords[21]) / 1000;

    curThirdinverter->AC_Voltage[0] = (float)pWords[22] / 10;	//L1���ѹ
    curThirdinverter->AC_Current[0] = (float)pWords[23] / 100;  //L1�����
    curThirdinverter->Grid_Frequency[0] = (float)pWords[24] / 100;  //L1��Ƶ��
    curThirdinverter->AC_Voltage[1] = (float)pWords[28] / 10;	//L2���ѹ
    curThirdinverter->AC_Current[1] = (float)pWords[29] / 100;  //L2�����
    curThirdinverter->Grid_Frequency[1] = (float)pWords[30] / 100;  //L2��Ƶ��
    curThirdinverter->AC_Voltage[2] = (float)pWords[34] / 10;	//L3���ѹ
    curThirdinverter->AC_Current[2] = (float)pWords[35] / 100;  //L3�����
    curThirdinverter->Grid_Frequency[2] = (float)pWords[36] / 100;  //L3��Ƶ��


    curThirdinverter->Daily_Energy  = (float)pWords[44] / 100;				         //���췢����
    curThirdinverter->Life_Energy   = (float)(pWords[49] *65536 + pWords[50]) / 100; //��ʷ������

    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//��ǰһ�ַ�����������һ�ֵķ�����
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy - Daily_Energy_last;
    }
    else
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy;
    }

    return 0;
}


/*****************************************************************************/
/* Function Name : GetData_SA_DeviceInfo_1                                   */
/* Function Description: Get DeviceInfo Data1                                */
/* date :2018-04-26                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
static int GetData_SA_DeviceInfo(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//��ȡ���ֽ�
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_OutWords(curThirdinverter->inverter_addr,
                          SA_DEVICEINFO_ADDRESS, SA_DEVICEINFO_LENGTH,
                          &nbReadWords,pWords);
    if(0 == ret)
    {
        //�豸���Խ���
        ResolveData_SA_DeviceInfo(curThirdinverter, pWords);
        return 0;
    }else
    {
        return -1;
    }
}

/*****************************************************************************/
/* Function Name :  GetData_SG_DeviceInfo                                    */
/* Function Description: Get SA Inverter Data                                */
/* date :2018-04-26                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int GetData_ThirdInverter_SA(inverter_third_info *curThirdinverter)
{
    int ret = 0;
    //��ȡ�豸����1
    ret = GetData_SA_DeviceInfo(curThirdinverter);
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

