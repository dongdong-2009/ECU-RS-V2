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
#include "usart485.h"

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
        strcpy(curThirdinverter->type,"60KTL");
    }else if(SG60KU == pWords[0])
    {
        strcpy(curThirdinverter->type,"60KU");
    }else if(SG30KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"30KTL-M");
    }else if(SG33KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"33KTL-M");
    }else if(SG36KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"36KTL-M");
    }else if(SG40KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"40KTL-M");
    }else if(SG50KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"50KTL-M");
    }else if(SG60KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"60KTL-M");
    }else if(SG60KU_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"60KU-M");
    }else if(SG49K5J == pWords[0])
    {
        strcpy(curThirdinverter->type,"49K5J");
    }else if(SG8KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"8KTL-M");
    }else if(SG10KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"10KTL-M");
    }else if(SG12KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"12KTL-M");
    }else if(SG80KTL == pWords[0])
    {
        strcpy(curThirdinverter->type,"80KTL");
    }else if(SG80KTL_M == pWords[0])
    {
        strcpy(curThirdinverter->type,"80KTL-M");
    }else if(SG80HV == pWords[0])
    {
        strcpy(curThirdinverter->type,"80HV");
    }else if(SG125HV == pWords[0])
    {
        strcpy(curThirdinverter->type,"125HV");
    }else
    {
        ;
    }
    curThirdinverter->Daily_Energy = (float)pWords[3] / 10;				//���췢����
    curThirdinverter->Life_Energy = pWords[4]+ pWords[5] *65536 ;	//��ʷ������
    curThirdinverter->Temperature = (float)((signed short)pWords[8]) / 10;	//�¶�
    if(pWords[11]== 0xFFFF)
        curThirdinverter->PV_Voltage[0] = 0;			//ֱ����ѹ1
    else
        curThirdinverter->PV_Voltage[0] = (float)pWords[11]	/ 10;			//ֱ����ѹ1

    if(pWords[12]== 0xFFFF)
        curThirdinverter->PV_Current[0] = 0;			//ֱ������1
    else
        curThirdinverter->PV_Current[0] = (float)pWords[12] / 10;			//ֱ������1

    if(pWords[13]== 0xFFFF)
        curThirdinverter->PV_Voltage[1] = 0;			//ֱ����ѹ2
    else
        curThirdinverter->PV_Voltage[1] = (float)pWords[13]	/ 10;			//ֱ����ѹ2

    if(pWords[14]== 0xFFFF)
        curThirdinverter->PV_Current[1] = 0;			//ֱ������2
    else
        curThirdinverter->PV_Current[1] = (float)pWords[14] / 10;			//ֱ������2

    if(pWords[15]== 0xFFFF)
        curThirdinverter->PV_Voltage[2] = 0;
    else
        curThirdinverter->PV_Voltage[2] = (float)pWords[15]	/ 10;			//ֱ����ѹ3

    if(pWords[16]== 0xFFFF)
        curThirdinverter->PV_Current[2] = 0;
    else
        curThirdinverter->PV_Current[2] = (float)pWords[16] / 10;			//ֱ������3

    curThirdinverter->PV_Power[0] = curThirdinverter->PV_Current[0] * curThirdinverter->PV_Voltage[0];
    curThirdinverter->PV_Power[1] = curThirdinverter->PV_Current[1] * curThirdinverter->PV_Voltage[1];
    curThirdinverter->PV_Power[2] = curThirdinverter->PV_Current[2] * curThirdinverter->PV_Voltage[2];

    //�ж������ѹ�����ߵ�ѹ
    if(2 == pWords[2])	//�ߵ�ѹ
    {
        curThirdinverter->AC_Voltage[0] = (float)pWords[19] / 10 / 1.73205;
        curThirdinverter->AC_Voltage[1] =  (float)pWords[20] / 10 / 1.73205;
        curThirdinverter->AC_Voltage[2] = (float)pWords[21] / 10 / 1.73205;

        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = (float)pWords[23] / 10;
        curThirdinverter->AC_Current[2] = (float)pWords[24] / 10;
        curThirdinverter->Grid_Frequency[0] = curThirdinverter->Grid_Frequency[1] = curThirdinverter->Grid_Frequency[2]  = (float)pWords[36] / 10;			//����Ƶ��

    }else if(1 == pWords[2])			//���ѹ
    {
        curThirdinverter->AC_Voltage[0] = (float)pWords[19] / 10;	//L1
        curThirdinverter->AC_Voltage[1] = (float)pWords[20] / 10;
        curThirdinverter->AC_Voltage[2] = (float)pWords[21] / 10;

        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = (float)pWords[23] / 10;
        curThirdinverter->AC_Current[2] = (float)pWords[24] / 10;
        curThirdinverter->Grid_Frequency[0] = curThirdinverter->Grid_Frequency[1] = curThirdinverter->Grid_Frequency[2]  = (float)pWords[36] / 10;			//����Ƶ��
    }else
    {
        curThirdinverter->AC_Voltage[0] = (float)pWords[19] / 10;
        curThirdinverter->AC_Voltage[1] = 0;
        curThirdinverter->AC_Voltage[2] = 0;
        curThirdinverter->AC_Current[0] = (float)pWords[22] / 10;
        curThirdinverter->AC_Current[1] = 0;
        curThirdinverter->AC_Current[2] = 0;
        curThirdinverter->Grid_Frequency[0] = (float)pWords[36] / 10;			//����Ƶ��
        curThirdinverter->Grid_Frequency[1] = curThirdinverter->Grid_Frequency[2]  = 0;
    }
    

    curThirdinverter->Reactive_Power = pWords[33]  + pWords[34]*65536;                        //�޹�����
    curThirdinverter->Active_Power = pWords[31] + pWords[32] *65536;                          //�й�����
    curThirdinverter->Power_Factor = (float)((signed short)pWords[35]) / 1000;                        //��������
    


    if(curThirdinverter->Daily_Energy > Daily_Energy_last)	//��ǰһ�ַ�����������һ�ֵķ�����
    {
        curThirdinverter->Current_Energy = curThirdinverter->Daily_Energy - Daily_Energy_last;
    }else
    {
        curThirdinverter->Current_Energy = 0;
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
    if(pWords[15]== 0xFFFF)
        curThirdinverter->PV_Voltage[3] = 0;
    else
        curThirdinverter->PV_Voltage[3] = (float)pWords[15]	/ 10;			//ֱ����ѹ4
    if(pWords[16]== 0xFFFF)
        curThirdinverter->PV_Current[3] = 0;			//ֱ������4
    else
        curThirdinverter->PV_Current[3] = (float)pWords[16] / 10;			//ֱ������4

    curThirdinverter->PV_Power[3] = curThirdinverter->PV_Current[3] * curThirdinverter->PV_Voltage[3];
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
    usart485_init(9600);
    //��ȡ�豸����1
    ret = GetData_SG_DeviceInfo_1(curThirdinverter);
    if(0 == ret)	//�豸���Բɼ��ɹ��������ɼ���������Ϣ
    {
        ret = GetData_SG_DeviceInfo_2(curThirdinverter);
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

        }else
        {
            curThirdinverter->third_status.communication_flag = 0;
            return -2;
        }

    }else
    {
        curThirdinverter->third_status.communication_flag = 0;
        return -1;
    }

}
