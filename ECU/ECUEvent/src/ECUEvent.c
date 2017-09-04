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
#include "RFM300H.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "file.h"

void ECUEvent_thread_entry(void* parameter)
{
	int ret = 0;
	rt_thread_delay(RT_TICK_PER_SECOND);
	
	while(1)
	{	
		//���WIFI�¼�
		process_WIFIEvent();
		
		//��ⰴ���¼�
		if(KEY_FormatWIFI_Event == 1)
		{
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event start\n");
			process_KEYEvent();
			KEY_FormatWIFI_Event = 0;
			SEGGER_RTT_printf(0,"KEY_FormatWIFI_Event end\n");
		}
		
		//WIFI��λ�¼�
		if(WIFI_RST_Event == 1)
		{
			SEGGER_RTT_printf(0,"WIFI_RST_Event start\n");
			ret = process_WIFI_RST();
			if(ret == 0)
				WIFI_RST_Event = 0;
			SEGGER_RTT_printf(0,"WIFI_RST_Event end\n");
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND/30);
	}	
}
