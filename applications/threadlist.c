/*****************************************************************************/
/* File      : threadlist.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <threadlist.h>
#include <board.h>
#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include "SEGGER_RTT.h"
#include "led.h"
#include "key.h"
#include "sys.h"
#include "usart5.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "variation.h"
#include "event.h"
#include "inverter.h"
#include "watchdog.h"
#include "serverfile.h"
#include "socket.h"
#include "rtc.h"


#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <stm32_eth.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif

#ifdef EEPROM		
#include "Flash_24L512.h"
#endif
#include "ds1302z_rtc.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
#ifdef THREAD_PRIORITY_LED
#include "led.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[500];
static struct rt_thread led_thread;
#endif

#ifdef THREAD_PRIORITY_LAN8720_RST
#include "lan8720rst.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t lan8720_rst_stack[400];
static struct rt_thread lan8720_rst_thread;
#endif

#ifdef THREAD_PRIORITY_UPDATE
#include "remoteUpdate.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t update_stack[2048];
static struct rt_thread update_thread;
#endif

#ifdef THREAD_PRIORITY_EVENT
#include "ECUEvent.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t event_stack[4096];
static struct rt_thread event_thread;
#endif 

#ifdef THREAD_PRIORITY_COMM
#include "ECUCOMM.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t comm_stack[4096];
static struct rt_thread comm_thread;
#endif 

#ifdef THREAD_PRIORITY_DATACOLLECT
#include "ECUCollect.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t collect_stack[4096];
static struct rt_thread collect_thread;
#endif 

#ifdef THREAD_PRIORITY_CLIENT
#include "ECUClient.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t client_stack[4096];
static struct rt_thread client_thread;
#endif 

#ifdef THREAD_PRIORITY_CONTROL_CLIENT
#include "ECUControl.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t control_stack[4096];
static struct rt_thread control_thread;
#endif 

#ifdef THREAD_PRIORITY_JSON_SERVER
#include "ECUJsonServer.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t json_server_stack[1024];
static struct rt_thread json_server_thread;
#endif 

#ifdef THREAD_PRIORITY_IDWRITE
#include "idwrite.h"
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t idwrite_stack[2048];
static struct rt_thread idwrite_thread;
#endif

ecu_info ecu;	//ecu相关信息
inverter_info inverterInfo[MAXINVERTERCOUNT] = {'\0'};	//rsd相关信息
unsigned char LED_Status = 0;
unsigned char LED_IDWrite_Status = 0;


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
extern void cpu_usage_init(void);
extern void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor);

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Device Init program entry                                               */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter[in]   unused                                                  */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void rt_init_thread_entry(void* parameter)
{
	{
		extern void rt_platform_init(void);
		rt_platform_init();
	}

	/* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* initialize the device file system */
	dfs_init();

	/* initialize the elm chan FatFS file system*/
	elm_init();
    
	/* mount flash fat partition 1 as root directory */
	if (dfs_mount("flash", "/", "elm", 0, 0) == 0)
	{
		rt_kprintf("File System initialized!\n");
	}
	else
	{
		rt_kprintf("File System initialzation failed!\n");
		dfs_mkfs("elm","flash");
		if (dfs_mount("flash", "/", "elm", 0, 0) == 0)
		{
			rt_kprintf("File System initialized!\n");
		}
		initPath();		
		rt_kprintf("PATH initialized!\n");
	}
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

	
#ifdef RT_USING_LWIP
  /* initialize eth interface */
  rt_hw_stm32_eth_init();

	/* initialize lwip stack */
	/* register ethernetif device */
	eth_system_device_init();

	/* initialize lwip system */
	lwip_system_init();
	
	SEGGER_RTT_printf(0,"TCP/IP initialized!\n");
	rt_kprintf("TCP/IP initialized!\n");
#endif

#ifdef RT_USING_FINSH
	/* initialize finsh */
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	/* initialize rtc */
	rt_hw_rtc_init();		
	cpu_usage_init();	
	
	KEY_Init();										//恢复出厂设置按键初始化
	rt_hw_ms_delay(200);
	EXTIX_Init();									//恢复出厂设置IO中断初始化
	
	//RFM_init();
	//RFM_off();
	rt_hw_led_init();
#ifdef USR_MODULE
	//uart5_init(57600);					//USR模块相应波特率 串口初始化
	uart5_init(115200);					//USR模块相应波特率 串口初始化
#endif

#ifdef RAK475_MODULE	
	uart5_init(115200);					//RAK475相应波特率 串口初始化
#endif 
	TIM2_Int_Init(14999,7199);    //心跳包超时事件定时器初始化
	WIFI_Reset();
	SEGGER_RTT_printf(0, "init OK \n");
	init_RecordMutex();
	initSocketArgs();
	sysDirDetection();
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   LED program entry                                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter[in]   unused                                                  */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
#ifdef THREAD_PRIORITY_LED
static void led_thread_entry(void* parameter)
{
	rt_uint8_t major,minor;
	/* Initialize led */
	rt_hw_led_init();
	rt_hw_watchdog_init();
	while (1)
    {
    	kickwatchdog();
		if(LED_Status == 0)
		{
			rt_hw_led_off();
		}else
		{
			rt_hw_led_on();
		}
		if(LED_IDWrite_Status == 1)
		{
			rt_hw_led_on();
		}
		rt_thread_delay( RT_TICK_PER_SECOND);
		cpu_usage_get(&major, &minor);
		//printf("CPU : %d.%d%\n", major, minor);
    }
}
#endif

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Lan8720 Reset program entry                                             */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter[in]   unused                                                  */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
#ifdef THREAD_PRIORITY_LAN8720_RST
static void lan8720_rst_thread_entry(void* parameter)
{
	int value;
	char Time[15] = {'\0'};
	while (1)
	{
		value = ETH_ReadPHYRegister(0x00, 0);
			
		if(0 == value)	//判断控制寄存器是否变为0  表示断开
		{
			//printf("reg 0:%x\n",value);
			rt_hw_lan8720_rst();
		}

		apstime(Time);
		if(!memcmp(&Time[8],"0200",4))
		{
			printf("reboot :%s\n",Time);
			reboot();
		}

			
      	rt_thread_delay( RT_TICK_PER_SECOND*50 );
	}

}
#endif

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Create Application ALL Tasks                                            */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void tasks_new(void)
{
	rt_err_t result;
	rt_thread_t tid;
	
	/* init init thread */
  tid = rt_thread_create("init",rt_init_thread_entry, RT_NULL,1024, THREAD_PRIORITY_INIT, 20);
	if (tid != RT_NULL) rt_thread_startup(tid);
	
#ifdef THREAD_PRIORITY_LED
  /* init led thread */
  result = rt_thread_init(&led_thread,"led",led_thread_entry,RT_NULL,(rt_uint8_t*)&led_stack[0],sizeof(led_stack),THREAD_PRIORITY_LED,5);
  if (result == RT_EOK)	rt_thread_startup(&led_thread);
#endif

#ifdef THREAD_PRIORITY_LAN8720_RST
  /* init LAN8720RST thread */
  result = rt_thread_init(&lan8720_rst_thread,"lanrst",lan8720_rst_thread_entry,RT_NULL,(rt_uint8_t*)&lan8720_rst_stack[0],sizeof(lan8720_rst_stack),THREAD_PRIORITY_LAN8720_RST,5);
  if (result == RT_EOK)	rt_thread_startup(&lan8720_rst_thread);
#endif

#ifdef THREAD_PRIORITY_UPDATE	
  /* init update thread */
	result = rt_thread_init(&update_thread,"update",remote_update_thread_entry,RT_NULL,(rt_uint8_t*)&update_stack[0],sizeof(update_stack),THREAD_PRIORITY_UPDATE,5);
  if (result == RT_EOK) rt_thread_startup(&update_thread);
#endif

#ifdef THREAD_PRIORITY_IDWRITE	
  /* init idwrite thread */
	result = rt_thread_init(&idwrite_thread,"idwrite",idwrite_thread_entry,RT_NULL,(rt_uint8_t*)&idwrite_stack[0],sizeof(idwrite_stack),THREAD_PRIORITY_IDWRITE,5);
  if (result == RT_EOK) rt_thread_startup(&idwrite_thread);
#endif

#ifdef THREAD_PRIORITY_JSON_SERVER
  /* init JSON Server thread */
	result = rt_thread_init(&json_server_thread,"json",ECUJsonServer_thread_entry,RT_NULL,(rt_uint8_t*)&json_server_stack[0],sizeof(json_server_stack),THREAD_PRIORITY_JSON_SERVER,5);
  if (result == RT_EOK)	rt_thread_startup(&json_server_thread);
#endif

#ifdef THREAD_PRIORITY_EVENT
  /* init Event thread */
  result = rt_thread_init(&event_thread,"event",ECUEvent_thread_entry,RT_NULL,(rt_uint8_t*)&event_stack[0],sizeof(event_stack),THREAD_PRIORITY_EVENT,5);
  if (result == RT_EOK)	rt_thread_startup(&event_thread);
#endif

#ifdef THREAD_PRIORITY_COMM
  /* init Communication thread */
  result = rt_thread_init(&comm_thread,"comm",ECUComm_thread_entry,RT_NULL,(rt_uint8_t*)&comm_stack[0],sizeof(comm_stack),THREAD_PRIORITY_COMM,5);
  if (result == RT_EOK)	rt_thread_startup(&comm_thread);
#endif
	
#ifdef THREAD_PRIORITY_DATACOLLECT
  /* init Communication thread */
  result = rt_thread_init(&collect_thread,"collect",ECUCollect_thread_entry,RT_NULL,(rt_uint8_t*)&collect_stack[0],sizeof(collect_stack),THREAD_PRIORITY_DATACOLLECT,5);
  if (result == RT_EOK)	rt_thread_startup(&collect_thread);
#endif
#ifdef THREAD_PRIORITY_CLIENT
  /* init Client thread */
  result = rt_thread_init(&client_thread,"client",ECUClient_thread_entry,RT_NULL,(rt_uint8_t*)&client_stack[0],sizeof(client_stack),THREAD_PRIORITY_CLIENT,5);
  if (result == RT_EOK)	rt_thread_startup(&client_thread);
#endif
#ifdef THREAD_PRIORITY_CONTROL_CLIENT
  /* init Control thread */
  result = rt_thread_init(&control_thread,"control",ECUControl_thread_entry,RT_NULL,(rt_uint8_t*)&control_stack[0],sizeof(control_stack),THREAD_PRIORITY_CONTROL_CLIENT,5);
  if (result == RT_EOK)	rt_thread_startup(&control_thread);
#endif


}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Reset Application Tasks                                                 */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   type[In]:                                                               */
/*            TYPE_LED           :  Reset Led Task                           */
/*            TYPE_LANRST        :  Reset Lan8720 reset Task                 */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void restartThread(threadType type)
{
	rt_err_t result;
	switch(type)
	{
#ifdef THREAD_PRIORITY_LED
		case TYPE_LED:
			rt_thread_detach(&led_thread);
			/* init led thread */
			result = rt_thread_init(&led_thread,"led",led_thread_entry,RT_NULL,(rt_uint8_t*)&led_stack[0],sizeof(led_stack),THREAD_PRIORITY_LED,5);
			if (result == RT_EOK)	rt_thread_startup(&led_thread);
			break;
#endif 

#ifdef THREAD_PRIORITY_LAN8720_RST
		case TYPE_LANRST:
			rt_thread_detach(&lan8720_rst_thread);
			/* init LAN8720RST thread */
			result = rt_thread_init(&lan8720_rst_thread,"lanrst",lan8720_rst_thread_entry,RT_NULL,(rt_uint8_t*)&lan8720_rst_stack[0],sizeof(lan8720_rst_stack),THREAD_PRIORITY_LAN8720_RST,5);
			if (result == RT_EOK)	rt_thread_startup(&lan8720_rst_thread);
			break;
#endif 		

#ifdef THREAD_PRIORITY_UPDATE
		case TYPE_UPDATE:
			rt_thread_detach(&update_thread);
		  /* init update thread */
			result = rt_thread_init(&update_thread,"update",remote_update_thread_entry,RT_NULL,(rt_uint8_t*)&update_stack[0],sizeof(update_stack),THREAD_PRIORITY_UPDATE,5);
			if (result == RT_EOK)	rt_thread_startup(&update_thread);
			break;
#endif

#ifdef THREAD_PRIORITY_COMM
		case TYPE_COMM:
			rt_thread_detach(&comm_thread);
		  /* init comm thread */
		  result = rt_thread_init(&comm_thread,"comm",ECUComm_thread_entry,RT_NULL,(rt_uint8_t*)&comm_stack[0],sizeof(comm_stack),THREAD_PRIORITY_COMM,5);
		  if (result == RT_EOK) rt_thread_startup(&comm_thread);
			break;
#endif
#ifdef THREAD_PRIORITY_DATACOLLECT
		case TYPE_DATACOLLECT:
			rt_thread_detach(&collect_thread);
			/* init Communication thread */
			result = rt_thread_init(&collect_thread,"collect",ECUCollect_thread_entry,RT_NULL,(rt_uint8_t*)&collect_stack[0],sizeof(collect_stack),THREAD_PRIORITY_DATACOLLECT,5);
			if (result == RT_EOK) rt_thread_startup(&collect_thread);

			break;
#endif

		default:
			break;
	}
}

//定时器启动
rt_timer_t threadRestarttimer;	//线程重启定时器
threadType threadRestartType;



//定时器超时函数   复位超时
static void threadRestartTimeout(void* parameter)
{
	restartThread(threadRestartType);
}

//定时重启某个线程
void threadRestartTimer(int timeout,threadType Type)			
{
	threadRestartType = Type;
	threadRestarttimer = rt_timer_create("Restart", /* 定时器名字为 read */
					threadRestartTimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (threadRestarttimer != RT_NULL) rt_timer_start(threadRestarttimer);
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void restart(int type)
{
	restartThread((threadType)type);
}
FINSH_FUNCTION_EXPORT(restart, eg:restart());

#include "arch/sys_arch.h"
void dhcpreset(void)
{
	dhcp_reset();
}
FINSH_FUNCTION_EXPORT(dhcpreset, eg:dhcpreset());

void teststaticIP(void)
{
	IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
	IPAddr.IP1 = 192;
	IPAddr.IP2 = 168;
	IPAddr.IP3 = 1;
	IPAddr.IP4 = 192;

	MSKAddr.IP1 = 255;
	MSKAddr.IP2 = 255;
	MSKAddr.IP3 = 255;
	MSKAddr.IP4 = 0;

	GWAddr.IP1 = 192;
	GWAddr.IP2 = 168;
	GWAddr.IP3 = 1;
	GWAddr.IP4 = 1;
	
	DNS1Addr.IP1 = 0;
	DNS1Addr.IP2 = 0;
	DNS1Addr.IP3 = 0;
	DNS1Addr.IP4 = 0;

	DNS1Addr.IP1 = 0;
	DNS1Addr.IP2 = 0;
	DNS1Addr.IP3 = 0;
	DNS1Addr.IP4 = 0;
	
	StaticIP(IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);	

}
FINSH_FUNCTION_EXPORT(teststaticIP, eg:teststaticIP());

#endif
