/*****************************************************************************/
/* File      : third_inverter_SF.c                                           */
/*  History:                                                                 */
/*  Date       * Author          * Changes                                   */
/*  2018-05-8 * miaoweijie  * Creation of the file                           */
/*             *                 *                                           */
/*****************************************************************************/

/*  Include Files */
#include "third_inverter_SF.h"
#include "third_inverter.h"
#include "modbus.h"
#include "string.h"
#include "stdio.h"
#include "usart485.h"

/*  Definitions */
#define SF_DEVICEINFO_ADDRESS	0006
#define SF_DEVICEINFO_LENGTH	20 

extern ecu_info ecu;


/*****************************************************************************/
/* Function Name :  ResolveData_SF_DeviceInfo                                */
/* Function Description: Get SA Inverter Data                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
static void ResolveData_SF_DeviceInfo(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    float Daily_Energy_last = curThirdinverter->Daily_Energy;

	curThirdinverter->PV_Voltage[0] = (float)pWords[0] / 10;	       //直流电压1
	curThirdinverter->PV_Current[0] = (float)pWords[1] / 100;          //直流电流1
	curThirdinverter->PV_Voltage[1] = (float)pWords[2] / 10;	       //直流电压2
	curThirdinverter->PV_Current[1] = (float)pWords[3] / 100;          //直流电流2
    curThirdinverter->PV_Power[0]   = curThirdinverter->PV_Current[0] * curThirdinverter->PV_Voltage[0];
    curThirdinverter->PV_Power[1]   = curThirdinverter->PV_Current[1] * curThirdinverter->PV_Voltage[1];

    curThirdinverter->Active_Power      = (float)pWords[6] / 100;             //有功功率
	curThirdinverter->Grid_Frequency[0] = (float)pWords[8] / 100;			  //电网频率
	curThirdinverter->Grid_Frequency[1] = curThirdinverter->Grid_Frequency[2]  = 0;
	curThirdinverter->AC_Voltage[0] = (float)pWords[9] / 10; 	   //相电压L1
	curThirdinverter->AC_Current[0] = (float)pWords[10] / 100; 	   //相电流L1
	curThirdinverter->AC_Voltage[1] = (float)pWords[11] / 10; 	   //相电压L2
	curThirdinverter->AC_Current[1] = (float)pWords[12] / 100; 	   //相电流L2
	curThirdinverter->AC_Voltage[2] = (float)pWords[13] / 10; 	   //相电压L3
	curThirdinverter->AC_Current[2] = (float)pWords[14] / 100; 	   //相电流L3

    curThirdinverter->Life_Energy   = (float)(pWords[15] *65536 + pWords[16]);   //历史发电量
	curThirdinverter->Daily_Energy  = (float)pWords[19] / 100; 	   //当天发电量

    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//当前一轮发电量大于上一轮的发电量
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
/* Function Name : GetData_SF_DeviceInfo                                     */
/* Function Description: Get DeviceInfo Data1                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
static int GetData_SF_DeviceInfo(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//读取到字节
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_OutWords(curThirdinverter->inverter_addr,
                          SF_DEVICEINFO_ADDRESS, SF_DEVICEINFO_LENGTH,
                          &nbReadWords,pWords);
    if(0 == ret)
    {
        //设备属性解析
        ResolveData_SF_DeviceInfo(curThirdinverter, pWords);
        return 0;
    }else
    {
        return -1;
    }
}

/*****************************************************************************/
/* Function Name :  GetData_ThirdInverter_SF                                 */
/* Function Description: Get HW Inverter Data                                */
/* date :2018-05-08                                                          */
/* Parameters:  curThirdinverter[in/out] third Inverter Point                */
/* Return Values:   request result                                           */
/*                                                                           */
/*****************************************************************************/
int GetData_ThirdInverter_SF(inverter_third_info *curThirdinverter)
{
    int ret = 0;
    //获取设备属性1
    
    usart485_init(get_ThirdBaudRate(curThirdinverter->cBaudrate));
    ret = GetData_SF_DeviceInfo(curThirdinverter);
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



