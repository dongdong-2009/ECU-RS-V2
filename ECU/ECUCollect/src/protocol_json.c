#include "protocol_json.h"
#include "cJSON.h"
#include "string.h"
#include "stdio.h"
#include "rtthread.h"
#include <stdlib.h>
#include "serverfile.h"

extern inverter_third_info thirdInverterInfo[MAX_THIRD_INVERTER_COUNT];
extern ecu_info ecu;


void protocol_SG(cJSON *ecu_data_root)
{
    int i = 0;
    char buff[32] = {'\0'};
    int SGJsonFlag = 0;
    cJSON *inverter_addecuid_root,*inverter_data_roots;
    inverter_third_info * curThirdinverter = thirdInverterInfo;
    for(i=0; (i<MAX_THIRD_INVERTER_COUNT)&&((int)strlen(curThirdinverter->inverterid) > 0); i++, curThirdinverter++)
    {
        if(!memcmp("SG001",curThirdinverter->factory,5))
        {

            if(curThirdinverter->third_status.communication_flag == 1)	//能通讯上
            {
                cJSON *inverter_data_root,*Alternating_Current_root,
                        *channel_data_root,*channel_data,*Alternating_Current;
                if(0 == SGJsonFlag)
                {
                    cJSON_AddItemToObject(ecu_data_root, "SG001", inverter_addecuid_root = cJSON_CreateObject());
                    cJSON_AddStringToObject(inverter_addecuid_root, "EID", ecu.ECUID12);
                    cJSON_AddItemToObject(inverter_addecuid_root, "D", inverter_data_roots = cJSON_CreateArray());

                    SGJsonFlag = 1;
                }
                cJSON_AddItemToArray(inverter_data_roots,inverter_data_root=cJSON_CreateObject());
                sprintf(buff,"SG%s",curThirdinverter->inverterid);
                cJSON_AddStringToObject(inverter_data_root, "SN", buff);
                cJSON_AddStringToObject(inverter_data_root, "PVN","4");
                cJSON_AddItemToObject(inverter_data_root, "P", channel_data_root = cJSON_CreateArray());
                cJSON_AddItemToArray(channel_data_root,channel_data=cJSON_CreateObject());
                cJSON_AddStringToObject(channel_data, "CH", "1");
                sprintf(buff,"%.1f",curThirdinverter->PV_Voltage[0]);
                cJSON_AddStringToObject(channel_data, "PVV", buff);
                sprintf(buff,"%.1f",curThirdinverter->PV_Current[0]);
                cJSON_AddStringToObject(channel_data, "PVC", buff);

                cJSON_AddItemToArray(channel_data_root,channel_data=cJSON_CreateObject());
                cJSON_AddStringToObject(channel_data, "CH", "2");
                sprintf(buff,"%.1f",curThirdinverter->PV_Voltage[1]);
                cJSON_AddStringToObject(channel_data, "PVV", buff);
                sprintf(buff,"%.1f",curThirdinverter->PV_Current[1]);
                cJSON_AddStringToObject(channel_data, "PVC", buff);

                cJSON_AddItemToArray(channel_data_root,channel_data=cJSON_CreateObject());
                cJSON_AddStringToObject(channel_data, "CH", "3");
                sprintf(buff,"%.1f",curThirdinverter->PV_Voltage[2]);
                cJSON_AddStringToObject(channel_data, "PVV", buff);
                sprintf(buff,"%.1f",curThirdinverter->PV_Current[2]);
                cJSON_AddStringToObject(channel_data, "PVC", buff);

                cJSON_AddItemToArray(channel_data_root,channel_data=cJSON_CreateObject());
                cJSON_AddStringToObject(channel_data, "CH", "4");
                sprintf(buff,"%.1f",curThirdinverter->PV_Voltage[3]);
                cJSON_AddStringToObject(channel_data, "PVV", buff);
                sprintf(buff,"%.1f",curThirdinverter->PV_Current[3]);
                cJSON_AddStringToObject(channel_data, "PVC", buff);

                cJSON_AddItemToObject(inverter_data_root, "A", Alternating_Current_root = cJSON_CreateArray());
                cJSON_AddItemToArray(Alternating_Current_root,Alternating_Current=cJSON_CreateObject());
                cJSON_AddStringToObject(Alternating_Current, "CH", "1");
                sprintf(buff,"%.1f",curThirdinverter->Grid_Frequency[0]);
                cJSON_AddStringToObject(Alternating_Current, "F", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Voltage[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACV", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Current[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACC", buff);


                cJSON_AddItemToArray(Alternating_Current_root,Alternating_Current=cJSON_CreateObject());
                cJSON_AddStringToObject(Alternating_Current, "CH", "2");
                sprintf(buff,"%.1f",curThirdinverter->Grid_Frequency[1]);
                cJSON_AddStringToObject(Alternating_Current, "F", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Voltage[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACV", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Current[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACC", buff);

                cJSON_AddItemToArray(Alternating_Current_root,Alternating_Current=cJSON_CreateObject());
                cJSON_AddStringToObject(Alternating_Current, "CH", "3");
                sprintf(buff,"%.1f",curThirdinverter->Grid_Frequency[2]);
                cJSON_AddStringToObject(Alternating_Current, "F", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Voltage[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACV", buff);
                sprintf(buff,"%.1f",curThirdinverter->AC_Current[0]);
                cJSON_AddStringToObject(Alternating_Current, "ACC", buff);

                sprintf(buff,"%.1f",curThirdinverter->Temperature);
                cJSON_AddStringToObject(inverter_data_root, "TE", buff);

                sprintf(buff,"%d",curThirdinverter->Reactive_Power);
                cJSON_AddStringToObject(inverter_data_root, "RP", buff);
                sprintf(buff,"%d",curThirdinverter->Active_Power);
                cJSON_AddStringToObject(inverter_data_root, "AP", buff);
                sprintf(buff,"%.3f",curThirdinverter->Power_Factor);
                cJSON_AddStringToObject(inverter_data_root, "PF", buff);
                sprintf(buff,"%.1f",curThirdinverter->Daily_Energy);
                cJSON_AddStringToObject(inverter_data_root, "DE", buff);
                sprintf(buff,"%.1f",curThirdinverter->Life_Energy);
                cJSON_AddStringToObject(inverter_data_root, "LE", buff);
                sprintf(buff,"%.1f",curThirdinverter->Current_Energy);
                cJSON_AddStringToObject(inverter_data_root, "CE", buff);


                //sprintf(buff,"%d",curThirdinverter->Input_Total_Power);
                //cJSON_AddStringToObject(inverter_data_root, "ITP", buff);
                //sprintf(buff,"%.1f",curThirdinverter->Month_Energy);
                //cJSON_AddStringToObject(inverter_data_root, "ME", buff);

            }


        }
    }
}


int protocol_JSON(char *sendcommanddatetime,inverter_third_info *firstThirdinverter,char *APSString)
{
    cJSON *ecu_data_root;

    char *out;
    ecu_data_root = cJSON_CreateObject();
    cJSON_AddStringToObject(ecu_data_root, "F", "0");
    cJSON_AddStringToObject(ecu_data_root, "EST", sendcommanddatetime);
    
    if(ecu.count > 0)
    {
        cJSON_AddStringToObject(ecu_data_root, "APS", APSString);
    }
    protocol_SG(ecu_data_root);



    out = cJSON_PrintUnformatted(ecu_data_root);

    cJSON_Delete(ecu_data_root);


    //memcpy(json,out,len);
    save_json_record(out,sendcommanddatetime);
    free(out);
    out = NULL;
    return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>

void testjson(void)
{
    protocol_JSON("20180426102100",thirdInverterInfo,"APS16XXXX");
}
FINSH_FUNCTION_EXPORT(testjson , testjson.)
#endif

