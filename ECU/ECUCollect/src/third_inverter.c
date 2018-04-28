/*****************************************************************************/
/* File      : third_inverter.c                                              */
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
#include "third_inverter.h"
#include "string.h"
#include "stdio.h"
#include "dfs_posix.h"
#include "debug.h"
#include "Serverfile.h"
#include "third_inverter_SG.h"
#include "third_inverter_SA.h"
#include "rtthread.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
inverter_third_info thirdInverterInfo[MAX_THIRD_INVERTER_COUNT];
extern ecu_info ecu;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Update third ID File                                                    */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void updateThirdID(void)	//更新第三方逆变器ID
{
    FILE *fp;
    int i;
    inverter_third_info *curThirdinverter = thirdInverterInfo;
    //id,addr,addrflag,factory,type,auto_getaddr_flag
    fp = fopen("/home/data/thirdid","w");
    if(fp)
    {
        curThirdinverter = thirdInverterInfo;
        for(i=0; (i<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdinverter->inverterid) > 0); i++, curThirdinverter++)
        {
            fprintf(fp,"%s,%d,%d,%s,%s,%d\n",curThirdinverter->inverterid,curThirdinverter->inverter_addr,
                    curThirdinverter->third_status.inverter_addr_flag,curThirdinverter->factory,curThirdinverter->type,curThirdinverter->third_status.autoget_addr);

        }
        fclose(fp);
    }
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get third ID from File                                                  */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   firstThirdinverter[in/out]   third Inverter struct point                */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   third Inverter NUM                                                      */
/*****************************************************************************/
int get_ThirdID_from_file(inverter_third_info *firstThirdinverter)
{
    int i,j,sameflag;
    inverter_third_info *thirdinverter = firstThirdinverter;
    inverter_third_info *curThirdinverter = firstThirdinverter;
    char list[6][32];
    char data[200];
    int num =0;
    FILE *fp;

    fp = fopen("/home/data/thirdid", "r");
    if(fp)
    {
        while(NULL != fgets(data,200,fp))
        {
            print2msg(ECU_DBG_COMM,"ID",data);
            memset(list,0,sizeof(list));
            splitString(data,list);
            printf("%s,%s,%s,%s,%s,%s\n",list[0],list[1],list[2],list[3],list[4],list[5]);
            //判断是否存在该逆变器
            curThirdinverter = firstThirdinverter;
            sameflag=0;
            for(j=0; ((j<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdinverter->inverterid) > 0)); j++)
            {
                if(!memcmp(list[0],curThirdinverter->inverterid,strlen(list[0])))
                    sameflag = 1;
                curThirdinverter++;
            }
            if(sameflag == 1)
            {
                continue;
            }

            strcpy(thirdinverter->inverterid, list[0]);

            if(0==strlen(list[1]))
            {
                thirdinverter->inverter_addr = 0;		//获取MODBUS从机地址
            }
            else
            {
                thirdinverter->inverter_addr = atoi(list[1]);
            }
            if(0==strlen(list[2]))
            {
                thirdinverter->third_status.inverter_addr_flag = 0;		//逆变器是否已经绑定地址
            }
            else
            {
                thirdinverter->third_status.inverter_addr_flag = atoi(list[2]);
            }

            strcpy(thirdinverter->factory, list[3]);
            strcpy(thirdinverter->type, list[4]);
            if(0==strlen(list[5]))
            {
                thirdinverter->third_status.autoget_addr = 0;		//逆变器是否已经绑定地址
            }
            else
            {
                thirdinverter->third_status.autoget_addr = atoi(list[5]);
            }


            thirdinverter++;
            num++;
            if(num >= MAX_THIRD_INVERTER_COUNT)
            {
                break;
            }
        }
        fclose(fp);
    }


    thirdinverter = firstThirdinverter;
    printmsg(ECU_DBG_COMM,"--------------");
    for(i=1; i<=num; i++, thirdinverter++)
        printdecmsg(ECU_DBG_COMM,thirdinverter->inverterid, thirdinverter->inverter_addr);
    printmsg(ECU_DBG_COMM,"--------------");
    printdecmsg(ECU_DBG_COMM,"total", num);


    thirdinverter = firstThirdinverter;
    printmsg(ECU_DBG_COMM,"--------------");

    return num;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   add GetData_ThirdInverter Function point                                */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   firstThirdinverter[in/out]   third Inverter struct point                */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void addGetData_ThirdInverter(inverter_third_info *firstThirdinverter)
{
    int i = 0;
    inverter_third_info *curThirdInverter = firstThirdinverter;
    for(i=0; ((i<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdInverter->inverterid) > 0)); i++,curThirdInverter++)
    {
        if(!memcmp(curThirdInverter->factory,"SG",2))
        {
            curThirdInverter->GetData_ThirdInverter = GetData_ThirdInverter_SG;
		}
		else if(!memcmp(curThirdInverter->factory,"SA",2))
		{
			curThirdInverter->GetData_ThirdInverter = GetData_ThirdInverter_SA;
		}
		else
		{
		    curThirdInverter->GetData_ThirdInverter = NULL;
		}
	}
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Init third Inverter                                                     */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   firstThirdinverter[in/out]   third Inverter struct point                */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void init_Third_Inverter(inverter_third_info *thirdInverter)
{
    int i = 0,j = 0;
    inverter_third_info *curThirdInverter = thirdInverter;

    //初始化第三方逆变器结构体
    for(i=0; i<MAX_THIRD_INVERTER_COUNT; i++, curThirdInverter++)
    {
        memset(curThirdInverter->inverterid,0x00,MAX_THIRD_INVERTERID_LEN);

        curThirdInverter->inverter_addr = 0;
        memset(curThirdInverter->factory,0x00,MAX_FACTORY_LEN);
        memset(curThirdInverter->type,0x00,MAX_TYPE_LEN);
        curThirdInverter->third_status.inverter_addr_flag =0;
        curThirdInverter->third_status.autoget_addr = 0;
        curThirdInverter->third_status.communication_flag = 0 ;

        for(j = 0;j< MAX_PV_NUM;j++)
        {
            curThirdInverter->PV_Voltage[j] = 0;
        }
        for(j = 0;j< MAX_PV_NUM;j++)
        {
            curThirdInverter->PV_Current[j] = 0;
        }
        for(j = 0;j < MAX_AC_NUM;j++)
        {
            curThirdInverter->AC_Voltage[j] = 0;
        }
        for(j = 0;j < MAX_AC_NUM;j++)
        {
            curThirdInverter->AC_Current[j] = 0;
        }
        curThirdInverter->Grid_Frequency[0] = 0;
        curThirdInverter->Grid_Frequency[1] = 0;
        curThirdInverter->Grid_Frequency[2] = 0;
        curThirdInverter->Temperature = 0;
        curThirdInverter->Reactive_Power = 0;
        curThirdInverter->Active_Power = 0;
        curThirdInverter->Power_Factor = 0;
        curThirdInverter->Daily_Energy = 0;
        curThirdInverter->Life_Energy = 0;
        curThirdInverter->Current_Energy = 0;

        curThirdInverter	->GetData_ThirdInverter = NULL;
    }
    //从文件中读取第三方ID信息
    get_ThirdID_from_file(thirdInverterInfo);
    //根据第三方逆变器的型号，选择对应的数据采集函数
    addGetData_ThirdInverter(thirdInverterInfo);
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Get third Inverter Data                                                 */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   firstThirdinverter[in/out]   third Inverter struct point                */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void getAllThirdInverterData(void)
{
    int i = 0;
    inverter_third_info *curThirdInverter = thirdInverterInfo;
    ecu.thirdCommNum = 0;
    for(i=0; ((i<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdInverter->inverterid) > 0)); i++,curThirdInverter++)
    {
        if((curThirdInverter->GetData_ThirdInverter))
        {
            if(0 == curThirdInverter->GetData_ThirdInverter(curThirdInverter))
            {
                Debug_ThirdInverter_info(curThirdInverter);
                ecu.thirdCommNum++;
            }
        }

    }
}

void Debug_ThirdInverter_info(inverter_third_info *curThirdinverter)
{
    printf("third ID:%s inverter_addr:%d factory:%s type:%s \n",curThirdinverter->inverterid,curThirdinverter->inverter_addr,curThirdinverter->factory,curThirdinverter->type);
    printf("inverter_addr_flag:%d autoget_addr:%d communication_flag:%d \n",curThirdinverter->third_status.inverter_addr_flag,curThirdinverter->third_status.autoget_addr,curThirdinverter->third_status.communication_flag);
    printf("PV_Voltage:%f %f %f %f \n",curThirdinverter->PV_Voltage[0],curThirdinverter->PV_Voltage[1],curThirdinverter->PV_Voltage[2],curThirdinverter->PV_Voltage[3]);
    printf("PV_Current:%f %f %f %f \n",curThirdinverter->PV_Current[0],curThirdinverter->PV_Current[1],curThirdinverter->PV_Current[2],curThirdinverter->PV_Current[3]);
    printf("AC_Voltage:%f %f %f \n",curThirdinverter->AC_Voltage[0],curThirdinverter->AC_Voltage[1],curThirdinverter->AC_Voltage[2]);
    printf("AC_Current:%f %f %f \n",curThirdinverter->AC_Current[0],curThirdinverter->AC_Current[1],curThirdinverter->AC_Current[2]);
    printf("Grid_Frequency:%f %f %f\n",curThirdinverter->Grid_Frequency[0],curThirdinverter->Grid_Frequency[1],curThirdinverter->Grid_Frequency[2]);
    printf("Temperature:%f \n",curThirdinverter->Temperature);
    printf("Reactive_Power:%d ",curThirdinverter->Reactive_Power);
    printf("Active_Power:%d \n",curThirdinverter->Active_Power);
    printf("Power_Factor:%f \n",curThirdinverter->Power_Factor);
    printf("Daily_Energy:%f \n",curThirdinverter->Daily_Energy);
    printf("Life_Energy:%f \n",curThirdinverter->Life_Energy);
    printf("Current_Energy:%f \n",curThirdinverter->Current_Energy);

}

#ifdef RT_USING_FINSH
#include <finsh.h>
void initThird(void)
{
    init_Third_Inverter(thirdInverterInfo);
}
FINSH_FUNCTION_EXPORT(initThird , initThird.)

void get_Third(void)
{
    getAllThirdInverterData();
}
FINSH_FUNCTION_EXPORT(get_Third , get_Third.)
#endif
