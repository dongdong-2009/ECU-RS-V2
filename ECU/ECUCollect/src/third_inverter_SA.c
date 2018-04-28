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
    //汇流板电路

    curThirdinverter->PV_Voltage[0] = (float)pWords[7]	/ 10;			//直流电压1
    curThirdinverter->PV_Current[0] = (float)pWords[8]  / 100;			//直流电流1
    curThirdinverter->PV_Voltage[1] = (float)pWords[10]	/ 10;			//直流电压2
    curThirdinverter->PV_Current[1] = (float)pWords[11] / 100;			//直流电流2
    curThirdinverter->PV_Voltage[2] = (float)pWords[13]	/ 10;			//直流电压3
    curThirdinverter->PV_Current[2] = (float)pWords[14] / 100;			//直流电流3
    curThirdinverter->Temperature   = (float)((signed short)pWords[17]) / 10;	//温度

    curThirdinverter->Active_Power  = pWords[19];                       //有功功率
    curThirdinverter->Reactive_Power = pWords[20];                      //无功功率
    curThirdinverter->Power_Factor  = (float)((signed short)pWords[21]) / 1000;

    curThirdinverter->AC_Voltage[0] = (float)pWords[22] / 10;	//L1相电压
    curThirdinverter->AC_Current[0] = (float)pWords[23] / 100;  //L1相电流
    curThirdinverter->Grid_Frequency[0] = (float)pWords[24] / 100;  //L1相频率
    curThirdinverter->AC_Voltage[1] = (float)pWords[28] / 10;	//L2相电压
    curThirdinverter->AC_Current[1] = (float)pWords[29] / 100;  //L2相电流
    curThirdinverter->Grid_Frequency[1] = (float)pWords[30] / 100;  //L2相频率
    curThirdinverter->AC_Voltage[2] = (float)pWords[34] / 10;	//L3相电压
    curThirdinverter->AC_Current[2] = (float)pWords[35] / 100;  //L3相电流
    curThirdinverter->Grid_Frequency[2] = (float)pWords[36] / 100;  //L3相频率


    curThirdinverter->Daily_Energy  = (float)pWords[44] / 100;				         //当天发电量
    curThirdinverter->Life_Energy   = (float)(pWords[49] *65536 + pWords[50]) / 100; //历史发电量

    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//当前一轮发电量大于上一轮的发电量
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
    unsigned char nbReadWords = 0;	//读取到字节
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_OutWords(curThirdinverter->inverter_addr,
                          SA_DEVICEINFO_ADDRESS, SA_DEVICEINFO_LENGTH,
                          &nbReadWords,pWords);
    if(0 == ret)
    {
        //设备属性解析
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
    //获取设备属性1
    ret = GetData_SA_DeviceInfo(curThirdinverter);
    if(0 == ret)
    {
        //判断addrflag绑定标志是否变化
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

