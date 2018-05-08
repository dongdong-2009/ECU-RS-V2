#ifndef __THREADLIST_H
#define __THREADLIST_H
/*****************************************************************************/
/* File      : threadlist.h                                                 */
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
#include <rtthread.h>
#include "arch/sys_arch.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define WIFI_USE 
//restartThread parameter
typedef enum THREADTYPE {
	TYPE_LED = 1,
	TYPE_LANRST = 2,
	TYPE_UPDATE = 3,
	TYPE_COMM = 4,
	TYPE_DATACOLLECT = 5,
	TYPE_TRINASOLAR = 6
}threadType;

#define ALARM_RECORD_ON

//WIFI 模块使用宏
#if 1
//使用有人模块
#define WIFI_MODULE_TYPE	2
#define USR_MODULE 		

#endif

#ifdef USR_MODULE
//socket每次发送的字节数
#define SIZE_PER_SEND		2700
#endif 

#define ESP07S_AT_TEST_CYCLE	60
#define ESP07S_AT_TEST_FAILED_NUM 2

//网络通讯地址
#if 1
#define CLIENT_SERVER_DOMAIN			""		//"ecu.apsema.com"
#define CLIENT_SERVER_IP					"60.190.131.190"
#define CLIENT_SERVER_PORT1				8982
#define CLIENT_SERVER_PORT2				8982

#define CONTROL_SERVER_DOMAIN			""		//"ecu.apsema.com"
#define CONTROL_SERVER_IP					"60.190.131.190"
#define CONTROL_SERVER_PORT1			8981
#define CONTROL_SERVER_PORT2			8981

#define UPDATE_SERVER_DOMAIN				"ecu.apsema.com"
#define UPDATE_SERVER_IP					"60.190.131.190"
#define UPDATE_SERVER_PORT1				9219

#define TRINASOLAR_SERVER_DOMAIN		"XXX.trinasolar.com"
#define TRINASOLAR_SERVER_IP			"192.168.1.2"
#define TRINASOLAR_SERVER_Port			60000
#else

#define CLIENT_SERVER_DOMAIN		""
//#define CLIENT_SERVER_IP			"139.168.200.158"
#define CLIENT_SERVER_IP			"60.190.131.190"

#define CLIENT_SERVER_PORT1			8982
#define CLIENT_SERVER_PORT2			8982

#define CONTROL_SERVER_DOMAIN		""
//#define CONTROL_SERVER_IP			"139.168.200.158"
#define CONTROL_SERVER_IP			"192.168.1.100"

#define CONTROL_SERVER_PORT1		8981
#define CONTROL_SERVER_PORT2		8981
#endif

//Thread Priority
//Init device thread priority
#define THREAD_PRIORITY_INIT							9
//LAN8720A Monitor thread priority
#define THREAD_PRIORITY_LAN8720_RST						11
//LED thread priority
#define THREAD_PRIORITY_LED               				9
//ID Writethread priority
#define THREAD_PRIORITY_IDWRITE							18
//Update thread priority
#define THREAD_PRIORITY_UPDATE							20
//LED thread priority
#define THREAD_PRIORITY_EVENT            				19			//其他相关事件线程
//JSON Server thread priority
#define THREAD_PRIORITY_JSON_SERVER						20
//LED thread priority
#define THREAD_PRIORITY_COMM              				24			//OPT700-RS通信线程
//Data Collection thread priority
#define THREAD_PRIORITY_DATACOLLECT       				25			//OPT700-RS相关数据采集
//Client thread priority
#define THREAD_PRIORITY_CLIENT       					26			//OPT700-RS数据上报
//Control Client thread priority
#define THREAD_PRIORITY_CONTROL_CLIENT  				26			//OPT700-RS远程控制

#define THREAD_PRIORITY_TRINASOLAR  				26			//天合客户端程序

//thread start time
#define START_TIME_EVENT									5
#define START_TIME_IDWRITE									1
#define START_TIME_UPDATE									4
#define START_TIME_COMM										5	
#define START_TIME_COLLECT								5			
#define START_TIME_CONTROL_CLIENT					20 	//150
#define START_TIME_CLIENT									30			//180
#define START_TIME_TRINASOLAR                                              10
/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//创建系统需要的线程
void tasks_new(void);
//复位线程
void restartThread(threadType type);
void threadRestartTimer(int timeout,threadType Type);


#endif
