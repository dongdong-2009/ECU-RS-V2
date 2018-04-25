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
    //��ȡ����
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
    curThirdinverter->Daily_Energy = (float)pWords[3] / 10;				//���췢����
    curThirdinverter->Life_Energy = pWords[4] *65536 + pWords[5];	//��ʷ������
    curThirdinverter->Temperature = (float)((signed short)pWords[8]) / 10;	//�¶�
    curThirdinverter->PV_Voltage[0] = (float)pWords[11]	/ 10;			//ֱ����ѹ1
    curThirdinverter->PV_Current[0] = (float)pWords[12] / 10;			//ֱ������1
    curThirdinverter->PV_Voltage[1] = (float)pWords[13]	/ 10;			//ֱ����ѹ2
    curThirdinverter->PV_Current[1] = (float)pWords[14] / 10;			//ֱ������2
    curThirdinverter->PV_Voltage[2] = (float)pWords[15]	/ 10;			//ֱ����ѹ3
    curThirdinverter->PV_Current[2] = (float)pWords[16] / 10;			//ֱ������3
    curThirdinverter->Input_Total_Power = pWords[17] *65536 + pWords[18];	//�����ܹ���
    //�ж������ѹ�����ߵ�ѹ
    if(2 == pWords[2])	//�ߵ�ѹ
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

    }else if(1 == pWords[2])			//���ѹ
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
    

    curThirdinverter->Reactive_Power = pWords[33] *65536 + pWords[34];                        //�޹�����
    curThirdinverter->Active_Power = pWords[31] *65536 + pWords[32];                          //�й�����
    curThirdinverter->Power_Factor = (float)((signed short)pWords[35]) / 1000;                        //��������
    curThirdinverter->Grid_Frequency = (float)pWords[36] / 10;			//����Ƶ��


    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//��ǰһ�ַ�����������һ�ֵķ�����
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
    curThirdinverter->PV_Voltage[3] = (float)pWords[15]	/ 10;			//ֱ����ѹ4
    curThirdinverter->PV_Current[3] = (float)pWords[16] / 10;			//ֱ������4
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
    //��16·�����������ֵ����Ӧ�ṹ��
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
    unsigned char nbReadWords = 0;	//��ȡ���ֽ�
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_DEVICEINFO_ADDRESS1-1),
                         SG_DEVICEINFO_LENGTH1, &nbReadWords,pWords);
    if(0 == ret)
    {
        //�豸���Խ���
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
    unsigned char nbReadWords = 0;	//��ȡ���ֽ�
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_DEVICEINFO_ADDRESS2-1),
                         SG_DEVICEINFO_LENGTH2, &nbReadWords,pWords);
    if(0 == ret)
    {
        //�豸���Խ���
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
    unsigned char nbReadWords = 0;	//��ȡ���ֽ�
    int ret = 0;
    unsigned short pWords[128] = {0};
    ret = read_N_InWords(curThirdinverter->inverter_addr, (SG_BUSBOARD_ARRRESS-1),
                         SG_BUSBOARD_LENGTH, &nbReadWords,pWords);
    if(0 == ret)
    {
        //������������Ϣ
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
    //��ȡ�豸����1
    ret = GetData_SG_DeviceInfo_1(curThirdinverter);
    if(0 == ret)	//�豸���Բɼ��ɹ��������ɼ���������Ϣ
    {
        ret = GetData_SG_DeviceInfo_2(curThirdinverter);
        if(0 == ret)
        {
            //��ȡ��������Ϣ
            ret = GetData_SG_BusBoard(curThirdinverter);
            if(0 == ret)	//��ȡ������ɹ�
            {
                //�ж�addrflag�󶨱�־�Ƿ�仯
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
