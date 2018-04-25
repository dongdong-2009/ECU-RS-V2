/*****************************************************************************/
/* File      : third_inverter_SG.c                                           */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-04-24 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "third_inverter_SG.h"
#include "modbus.h"
#include "string.h"
#include "stdio.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define SG_DEVICEINFO_ADDRESS1	5000
#define SG_DEVICEINFO_LENGTH1	100

#define SG_DEVICEINFO_ADDRESS2	5100
#define SG_DEVICEINFO_LENGTH2	100

#define SG_BUSBOARD_ARRRESS	7013
#define SG_BUSBOARD_LENGTH	16

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Resolve DeviceInfo Data1                                                */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter_third_info[in/out] third Inverter Point                        */
/*   pWords[int]  resolve short                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   resolve result                                                          */
/*****************************************************************************/
static int ResolveData_SG_DeviceInfo1(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    float Daily_Energy_last = curThirdinverter->Daily_Energy;
    //获取机型
    if(SG60KTL == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG60KTL");
    }else if(SG60KU == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG60KU");
    }else if(SG33KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG33KTL-M");
    }else if(SG40KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG40KTL-M");
    }else if(SG50KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG50KTL-M");
    }else if(SG60KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG60KTL-M");
    }else if(SG60KU_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG60KU-M");
    }else if(SG49K5J == pWords[0])
    {
        strcpy(curThirdinverter->type,"SG49K5J");
    }else
    {
        ;
    }
    curThirdinverter->Daily_Energy = (float)pWords[3] / 10;				//当天发电量
    curThirdinverter->Life_Energy = pWords[4] *65536 + pWords[5];	//历史发电量
    curThirdinverter->Temperature = (float)((signed short)pWords[8]) / 10;	//温度
    curThirdinverter->PV_Voltage[0] = (float)pWords[11]	/ 10;			//直流电压1
    curThirdinverter->PV_Current[0] = (float)pWords[12] / 10;			//直流电流1
    curThirdinverter->PV_Voltage[1] = (float)pWords[13]	/ 10;			//直流电压2
    curThirdinverter->PV_Current[1] = (float)pWords[14] / 10;			//直流电流2
    curThirdinverter->PV_Voltage[2] = (float)pWords[15]	/ 10;			//直流电压3
    curThirdinverter->PV_Current[2] = (float)pWords[16] / 10;			//直流电流3
    curThirdinverter->Input_Total_Power = pWords[17] *65536 + pWords[18];	//输入总功率
    //判断是相电压还是线电压
    if(2 == pWords[2])	//线电压
    {
        curThirdinverter->AC_Voltage[0] = -1;
        curThirdinverter->AC_Voltage[1] = -1;
        curThirdinverter->AC_Voltage[2] = -1;
        curThirdinverter->AB_Voltage = (float)pWords[19] / 10;
        curThirdinverter->BC_Voltage = (float)pWords[20] / 10;
        curThirdinverter->CA_Voltage = (float)pWords[21] / 10;
        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = (float)pWords[23] / 10;
        curThirdinverter->AC_Current[2] = (float)pWords[24] / 10;

    }else if(1 == pWords[2])			//相电压
    {
        curThirdinverter->AC_Voltage[0] = (float)pWords[19] / 10;	//L1
        curThirdinverter->AC_Voltage[1] = (float)pWords[20] / 10;
        curThirdinverter->AC_Voltage[2] = (float)pWords[21] / 10;
        curThirdinverter->AB_Voltage = -1;
        curThirdinverter->BC_Voltage = -1;
        curThirdinverter->CA_Voltage = -1;
        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = (float)pWords[23] / 10;
        curThirdinverter->AC_Current[2] = (float)pWords[24] / 10;
    }else			
    {
        curThirdinverter->AC_Voltage[0] = (float)pWords[19] / 10;
        curThirdinverter->AC_Voltage[1] = -1;
        curThirdinverter->AC_Voltage[2] = -1;
        curThirdinverter->AB_Voltage = -1;
        curThirdinverter->BC_Voltage = -1;
        curThirdinverter->CA_Voltage = -1;
        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = -1;
        curThirdinverter->AC_Current[2] = -1;
    }
    

    curThirdinverter->Reactive_Power = pWords[33] *65536 + pWords[34];                        //无功功率
    curThirdinverter->Active_Power = pWords[31] *65536 + pWords[32];                          //有功功率
    curThirdinverter->Power_Factor = (float)((signed short)pWords[35]) / 1000;                        //功率因数
    curThirdinverter->Grid_Frequency = (float)pWords[36] / 10;			//电网频率


    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//当前一轮发电量大于上一轮的发电量
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy - Daily_Energy_last;
    }else
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy;
    }

    return 0;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Resolve DeviceInfo Data2                                                */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter_third_info[in/out] third Inverter Point                        */
/*   pWords[int]  resolve short                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   resolve result                                                          */
/*****************************************************************************/
static int ResolveData_SG_DeviceInfo2(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    curThirdinverter->PV_Voltage[3] = (float)pWords[15]	/ 10;			//直流电压4
    curThirdinverter->PV_Current[3] = (float)pWords[16] / 10;			//直流电流4
    curThirdinverter->Month_Energy = (float)(pWords[33] *65536 + pWords[34]) / 10;
    return 0;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Resolve BUS Board                                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter_third_info[in/out] third Inverter Point                        */
/*   pWords[int]  resolve short                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   resolve result                                                          */
/*****************************************************************************/
static int ResolveData_SG_BUSBoard(inverter_third_info *curThirdinverter,unsigned short *pWords)
{
    int i = 0;
    //将16路汇流板电流赋值给对应结构体
    for(i = 0; i < MAX_CHANNEL_NUM ; i++)
    {
        curThirdinverter->Series_Current[i] = (float)pWords[i] / 100;
    }
    return 0;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get DeviceInfo Data1                                                    */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   curThirdinverter[in/out] third Inverter Point                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   request result                                                          */
/*****************************************************************************/
static int GetData_SG_DeviceInfo_1(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//读取到字节
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_DEVICEINFO_ADDRESS1-1),
                         SG_DEVICEINFO_LENGTH1, &nbReadWords,pWords);
    if(0 == ret)
    {
        //设备属性解析
        ResolveData_SG_DeviceInfo1(curThirdinverter,pWords);
        return 0;
    }else
    {
        return -1;
    }
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get DeviceInfo Data2                                                    */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   curThirdinverter[in/out] third Inverter Point                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   request result                                                          */
/*****************************************************************************/
static int GetData_SG_DeviceInfo_2(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//读取到字节
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_DEVICEINFO_ADDRESS2-1),
                         SG_DEVICEINFO_LENGTH2, &nbReadWords,pWords);
    if(0 == ret)
    {
        //设备属性解析
        ResolveData_SG_DeviceInfo2(curThirdinverter,pWords);
        return 0;
    }else
    {
        return -1;
    }
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get DeviceInfo Bus Board                                                */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   curThirdinverter[in/out] third Inverter Point                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   request result                                                          */
/*****************************************************************************/
static int GetData_SG_BusBoard(inverter_third_info *curThirdinverter)
{
    unsigned char nbReadWords = 0;	//读取到字节
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_BUSBOARD_ARRRESS-1),
                         SG_BUSBOARD_LENGTH, &nbReadWords,pWords);
    if(0 == ret)
    {
        //解析汇流板信息
        ResolveData_SG_BUSBoard(curThirdinverter,pWords);
        return 0;
    }else
    {
        return -1;
    }

}

static void Debug_SG_info(inverter_third_info *curThirdinverter)
{
    printf("third ID:%s inverter_addr:%d factory:%s type:%s \n",curThirdinverter->inverterid,curThirdinverter->inverter_addr,curThirdinverter->factory,curThirdinverter->type);
    printf("inverter_addr_flag:%d autoget_addr:%d communication_flag:%d \n",curThirdinverter->third_status.inverter_addr_flag,curThirdinverter->third_status.autoget_addr,curThirdinverter->third_status.communication_flag);
    printf("PV_Voltage:%f %f %f %f \n",curThirdinverter->PV_Voltage[0],curThirdinverter->PV_Voltage[1],curThirdinverter->PV_Voltage[2],curThirdinverter->PV_Voltage[3]);
    printf("PV_Current:%f %f %f %f \n",curThirdinverter->PV_Current[0],curThirdinverter->PV_Current[1],curThirdinverter->PV_Current[2],curThirdinverter->PV_Current[3]);
    printf("AC_Voltage:%f %f %f \n",curThirdinverter->AC_Voltage[0],curThirdinverter->AC_Voltage[1],curThirdinverter->AC_Voltage[2]);
    printf("AC_Current:%f %f %f \n",curThirdinverter->AC_Current[0],curThirdinverter->AC_Current[1],curThirdinverter->AC_Current[2]);
    printf("Grid_Frequency:%f ",curThirdinverter->Grid_Frequency);
    printf("Temperature:%f ",curThirdinverter->Temperature);
    printf("Reactive_Power:%d ",curThirdinverter->Reactive_Power);
    printf("Active_Power:%d \n",curThirdinverter->Active_Power);
    printf("Power_Factor:%f ",curThirdinverter->Power_Factor);
    printf("Daily_Energy:%f ",curThirdinverter->Daily_Energy);
    printf("Life_Energy:%f \n",curThirdinverter->Life_Energy);
    printf("Current_Energy:%f ",curThirdinverter->Current_Energy);
    printf("Month_Energy:%f ",curThirdinverter->Month_Energy);

}
/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get SG Inverter Data                                                    */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   curThirdinverter[in/out] third Inverter Point                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   request result                                                          */
/*****************************************************************************/
int GetData_ThirdInverter_SG(inverter_third_info *curThirdinverter)
{
    int ret = 0;
    //获取设备属性1
    ret = GetData_SG_DeviceInfo_1(curThirdinverter);
    if(0 == ret)	//设备属性采集成功，继续采集汇流板信息
    {
        ret = GetData_SG_DeviceInfo_2(curThirdinverter);
        if(0 == ret)
        {
            //获取汇流板信息
            ret = GetData_SG_BusBoard(curThirdinverter);
            if(0 == ret)	//读取汇流板成功
            {
                //判断addrflag绑定标志是否变化
                curThirdinverter->third_status.communication_flag = 1;
                if(curThirdinverter->third_status.inverter_addr_flag == 0)
                {
                    curThirdinverter->third_status.inverter_addr_flag = 1;
                    ecu.ThirdIDUpdateFlag = 1;
                }
		Debug_SG_info(curThirdinverter);
                return 0;
            }else
            {
                return -3;
            }
        }else
        {
            return -2;
        }

    }else
    {
        return -1;
    }

}
