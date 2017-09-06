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
}threadType;


//Thread Priority
//Init device thread priority
#define THREAD_PRIORITY_INIT							10
//LAN8720A Monitor thread priority
#define THREAD_PRIORITY_LAN8720_RST				11
//LED thread priority
#define THREAD_PRIORITY_LED               12
//LED thread priority
#define THREAD_PRIORITY_EVENT            	13			//其他相关事件线程
//LED thread priority
#define THREAD_PRIORITY_COMM              14			//OPT700-RS通信线程
//Data Collection thread priority
#define THREAD_PRIORITY_DATACOLLECT       15			//OPT700-RS相关数据采集
//Client thread priority
#define THREAD_PRIORITY_CLIENT       			17			//OPT700-RS数据上报
//Control Client thread priority
#define THREAD_PRIORITY_CONTROL_CLIENT  	16			//OPT700-RS远程控制

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//创建系统需要的线程
void tasks_new(void);
//复位线程
void restartThread(threadType type);

#endif
