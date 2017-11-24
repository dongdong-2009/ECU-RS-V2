#include "ECUEvent.h"
#include "event.h"
#include "led.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "Flash_24L512.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "arch/sys_arch.h"
#include "serverfile.h"
#include <lwip/netdb.h>
#include <lwip/sockets.h>


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

typedef struct IPConfig
{
	IP_t IPAddr;
	IP_t MSKAddr;
	IP_t GWAddr;
	IP_t DNS1Addr;
	IP_t DNS2Addr;
		
} IPConfig_t;


int getAddr(MyArray *array, int num,IPConfig_t *IPconfig)
{
	int i;
	ip_addr_t addr;
	for(i=0; i<num; i++){
		memset(&addr,0x00,sizeof(addr));
		if(!strlen(array[i].name))break;
		//IP地址
		if(!strcmp(array[i].name, "IPAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->IPAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->IPAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->IPAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->IPAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//掩码地址
		else if(!strcmp(array[i].name, "MSKAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->MSKAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->MSKAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->MSKAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->MSKAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//网关地址
		else if(!strcmp(array[i].name, "GWAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->GWAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->GWAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->GWAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->GWAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS1地址
		else if(!strcmp(array[i].name, "DNS1Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS1Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS1Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS1Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS1Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS2地址
		else if(!strcmp(array[i].name, "DNS2Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS2Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS2Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS2Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS2Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}	
	}
	return 0;
}



void ECUEvent_thread_entry(void* parameter)
{
	int ret = 0;
	MyArray array[5];
	int fileflag = 0; 
	IPConfig_t IPconfig;
	rt_thread_delay(RT_TICK_PER_SECOND);
	add_APP_functions();
	get_mac((unsigned char*)ecu.MacAddress);			//ECU 有线Mac地址
	fileflag = file_get_array(array, 5, "/config/staticIP.con");
	if(fileflag == 0)
	{
		getAddr(array, 5,&IPconfig);
		StaticIP(IPconfig.IPAddr,IPconfig.MSKAddr,IPconfig.GWAddr,IPconfig.DNS1Addr,IPconfig.DNS2Addr);
	}
	while(1)
	{	
		//检测WIFI事件
		process_WIFIEvent();
		
		//检测按键事件
		if(KEY_FormatWIFI_Event == 1)
		{
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event start\n");
			process_KEYEvent();
			KEY_FormatWIFI_Event = 0;
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event end\n");
		}
		
		//WIFI复位事件
		if(WIFI_RST_Event == 1)
		{
			SEGGER_RTT_printf(0,"WIFI_RST_Event start\n");
			ret = process_WIFI_RST();
			if(ret == 0)
				WIFI_RST_Event = 0;
			SEGGER_RTT_printf(0,"WIFI_RST_Event end\n");
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND/100);
	}	
}
