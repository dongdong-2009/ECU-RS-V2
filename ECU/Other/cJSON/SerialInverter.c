#include "cJSON.h"
#include "string.h"
#include "stdio.h"
#include "rtthread.h"
#include <stdlib.h>

int struct2json_sun001(char *json)
{
	int len = 0;
	cJSON *ecu_data_root, *inverter_addecuid_root,
	*inverter_data_roots,*inverter_data_root,
	*PV_Voltage_roots,*PV_Voltage_root,
	*PV_Current_roots,*PV_Current_root,
	*AC_Voltage_roots,*AC_Voltage_root,
	*AC_Current_roots,*AC_Current_root,
	*Series_Current_roots,*Series_Current_root;
	char *out;
	ecu_data_root = cJSON_CreateObject();
	cJSON_AddStringToObject(ecu_data_root, "Ecu_Send_Time", "20180420101019");

	cJSON_AddItemToObject(ecu_data_root, "SUN001", inverter_addecuid_root = cJSON_CreateObject());
	cJSON_AddStringToObject(inverter_addecuid_root, "ECU_ID", "1234567891");

	cJSON_AddItemToObject(inverter_addecuid_root, "Data", inverter_data_roots = cJSON_CreateArray());

	cJSON_AddItemToArray(inverter_data_roots,inverter_data_root=cJSON_CreateObject());
	cJSON_AddStringToObject(inverter_data_root, "SN", "12345678");
	cJSON_AddStringToObject(inverter_data_root, "Software", "12345678");

	cJSON_AddItemToObject(inverter_data_root, "PV_Voltage", PV_Voltage_roots = cJSON_CreateArray());
	cJSON_AddItemToArray(PV_Voltage_roots,PV_Voltage_root=cJSON_CreateObject());
	cJSON_AddStringToObject(PV_Voltage_root, "Channel", "1");
	cJSON_AddStringToObject(PV_Voltage_root, "Voltage", "26.7");

	cJSON_AddItemToObject(inverter_data_root, "PV_Current", PV_Voltage_roots = cJSON_CreateArray());
	cJSON_AddItemToArray(PV_Voltage_roots,PV_Voltage_root=cJSON_CreateObject());
	cJSON_AddStringToObject(PV_Voltage_root, "Channel", "1");
	cJSON_AddStringToObject(PV_Voltage_root, "Current", "4.9");

	cJSON_AddItemToObject(inverter_data_root, "AC_Voltage", PV_Voltage_roots = cJSON_CreateArray());
	cJSON_AddItemToArray(PV_Voltage_roots,PV_Voltage_root=cJSON_CreateObject());
	cJSON_AddStringToObject(PV_Voltage_root, "Channel", "1");
	cJSON_AddStringToObject(PV_Voltage_root, "Voltage", "26.7");

	cJSON_AddItemToObject(inverter_data_root, "AC_Current", PV_Voltage_roots = cJSON_CreateArray());
	cJSON_AddItemToArray(PV_Voltage_roots,PV_Voltage_root=cJSON_CreateObject());
	cJSON_AddStringToObject(PV_Voltage_root, "Channel", "1");
	cJSON_AddStringToObject(PV_Voltage_root, "Current", "4.9");

	cJSON_AddStringToObject(inverter_data_root, "Grid_Frequency", "49.8");
	cJSON_AddStringToObject(inverter_data_root, "Temperature", "21");
	cJSON_AddStringToObject(inverter_data_root, "Reactive_Power", "190");
	cJSON_AddStringToObject(inverter_data_root, "Acive_Power", "190");
	cJSON_AddStringToObject(inverter_data_root, "Power_Factor", "0.05");
	cJSON_AddStringToObject(inverter_data_root, "Daily_Energy", "3.1");
	cJSON_AddStringToObject(inverter_data_root, "Life_Energy", "19");
	cJSON_AddStringToObject(inverter_data_root, "Current_Energy", "19");
	cJSON_AddStringToObject(inverter_data_root, "Sys_Time", "201804201220");
	cJSON_AddStringToObject(inverter_data_root, "Single_Power", "190");
	cJSON_AddStringToObject(inverter_data_root, "Input_Total_Power", "1900");
	cJSON_AddStringToObject(inverter_data_root, "PV_Num", "10");

	cJSON_AddItemToObject(inverter_data_root, "Series_Current", Series_Current_roots = cJSON_CreateArray());
	cJSON_AddItemToArray(Series_Current_roots,Series_Current_root=cJSON_CreateObject());
	cJSON_AddStringToObject(Series_Current_root, "Channel", "1");
	cJSON_AddStringToObject(Series_Current_root, "Current", "2");

	cJSON_AddStringToObject(inverter_data_root, "MPPT_Input_Power", "1900");
	cJSON_AddStringToObject(inverter_data_root, "AB_Voltage", "26.7");
	cJSON_AddStringToObject(inverter_data_root, "BC_Voltage", "26.7");
	cJSON_AddStringToObject(inverter_data_root, "CA_Voltage", "26.7");
	cJSON_AddStringToObject(inverter_data_root, "Peak_Active_Power", "190");
	cJSON_AddStringToObject(inverter_data_root, "Hour_Energy", "0.3");
	cJSON_AddStringToObject(inverter_data_root, "Month_Energy", "23.5");
	cJSON_AddStringToObject(inverter_data_root, "Year_Energy", "235.7");
	cJSON_AddStringToObject(inverter_data_root, "Power_Hours", "1000");
	cJSON_AddStringToObject(inverter_data_root, "Status", "A0A0");
	cJSON_AddStringToObject(inverter_data_root, "Alarm", "A0A0");


	out = cJSON_PrintUnformatted(ecu_data_root);
	len = strlen(out);
	cJSON_Delete(ecu_data_root);

	printf("len=%d  %s\n", len ,out);

	memcpy(json,out,len);

	free(out); //±£¥Ê∫Û‘Ÿfree
	out = NULL;
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void testcJson(void)
{
	char *json = NULL;
	json = malloc(40000);
	struct2json_sun001(json);
	free(json);
}
FINSH_FUNCTION_EXPORT(testcJson, eg:testcJson());
#endif
