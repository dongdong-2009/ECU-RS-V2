/*****************************************************************************/
/*  File      : mycommand.c                                                  */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-13 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "rthw.h"
#include "threadlist.h"
#include <rtthread.h>
#include "serverfile.h"
#include "thftpapi.h"
#include <dfs_posix.h>
#include "InternalFlash.h"
#include "event.h"
#include "lan8720rst.h"
typedef struct 
{
	char remoteFile[50];
	char localFile[50];
}ftpPath_t;
//定时器
rt_timer_t reboottimer;
rt_timer_t ftpputtimer;
rt_timer_t ftpgettimer;
rt_timer_t factorytimer;

//定时器超时函数   复位超时
static void reboottimeout(void* parameter)
{
	reboot();
}

void reboot_timer(int timeout)			
{
	reboottimer = rt_timer_create("reboot", /* 定时器名字为 read */
					reboottimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (reboottimer != RT_NULL) rt_timer_start(reboottimer);
}

void Factory(void)
{
	char InternalECUID[13] = {'0'};
	char InternalMAC[18] = {'\0'};

	ReadPage(INTERNAL_FALSH_ID,InternalECUID,12);
	ReadPage(INTERNAL_FALSH_MAC,InternalMAC,17);
	dfs_mkfs("elm","flash");
	initPath();

	//写入ID 
	setECUID(InternalECUID);
	//写入MAC
	echo("/config/ecumac.con",InternalMAC);
}

unsigned char factory_flag = 0;
static void factorytimeout(void* parameter)
{
	factory_flag = 1;
}

void factory_timer(int timeout)
{
	factorytimer = rt_timer_create("factory", /* 定时器名字为 read */
					factorytimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (factorytimer != RT_NULL) rt_timer_start(factorytimer);
}

void rt_factory_thread_entry(void* parameter)
{
	factory_flag = 0;
	//开启定时器
	factory_timer(10);
	while(1)
	{
		if(factory_flag == 1)
		{
			factory_flag = 0;
			Factory();
			reboot();
			printf("Factory\n");
			break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	return;
}


ftpPath_t ftp_putPath;
unsigned char ftpput_flag = 0;
//定时器超时函数  上传数据超时
static void ftpputtimeout(void* parameter)
{
	ftpput_flag = 1;	
}

void ftpput_timer(int timeout)			//zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{	
	ftpputtimer = rt_timer_create("ftpput", /* 定时器名字为 read */
					ftpputtimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (ftpputtimer != RT_NULL) rt_timer_start(ftpputtimer);
}


void rt_ftpput_thread_entry(void* parameter)
{
	ftpput_flag = 0;
	//开启定时器
	ftpput_timer(10);
	while(1)
	{
		if(ftpput_flag == 1)
		{
			ftpput_flag = 0;
			putfile(ftp_putPath.remoteFile,ftp_putPath.localFile);
			break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	return;
}



ftpPath_t ftp_getPath;
unsigned char ftpget_flag = 0;
//定时器超时函数  上传数据超时
static void ftpgettimeout(void* parameter)
{
	ftpget_flag = 1;	
}

void ftpget_timer(int timeout)			//zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{	
	ftpputtimer = rt_timer_create("ftpget", /* 定时器名字为 read */
					ftpgettimeout, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (ftpputtimer != RT_NULL) rt_timer_start(ftpputtimer);
}


void rt_ftpget_thread_entry(void* parameter)
{
	ftpget_flag = 0;
	//开启定时器
	ftpget_timer(10);
	while(1)
	{
		if(ftpget_flag == 1)
		{
			ftpget_flag = 0;
			getfile(ftp_getPath.remoteFile,ftp_getPath.localFile);
			break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	return;
}


char responsesFlash(long long cap)
{
	if(cap == 0)
	{
		return 'A';
	}else if((cap>0)&&(cap<=1000))
	{
		return 'B';
	}else if((cap>1000)&&(cap<=2000))
	{
		return 'C';
	}else if((cap>2000)&&(cap<=3000))
	{
		return 'D';
	}else if((cap>3000)&&(cap<=4000))
	{
		return 'E';
	}else if((cap>4000)&&(cap<=5000))
	{
		return 'F';
	}else if((cap>5000)&&(cap<=6000))
	{
		return 'G';
	}else if((cap>6000)&&(cap<=7000))
	{
		return 'H';
	}else if((cap>7000)&&(cap<=8000))
	{
		return 'I';
	}else 
	{
		return 'J';
	}
}

char responsesNetwork(int status)
{
	if(status == 1)
	{
		return 'A';	//存在有线连接
	}else
	{
		return 'B';	//无线连接
	}
}

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
char mysystem(const char *command)
{
	char res = '4';

	print2msg(ECU_DBG_CONTROL_CLIENT,"Execute:",(char*) command);
	//需要的命令在此添加
	if(!memcmp(command,"reboot",6))
	{
		res = '0';
		//通过定时器复位程序
		reboot_timer(10);
	}else if(!memcmp(command,"restart UPDATE",14))
	{
		restartThread(TYPE_UPDATE);
		res = '0';
	}else if(!memcmp(command,"ftpput",6))	//上传数据
	{
		rt_thread_t tid;
		char sourcePath[50]={0x00},destPath[50]={0x00};
		//分割字符串为  命令 [本地源路径] [远程目标路径]
		splitSpace((char *)command,sourcePath,destPath);
		printf("cmd:%s\n",command);
		printf("%s,%s,%s\n","ftpput",sourcePath,destPath);
		memcpy(ftp_putPath.localFile,sourcePath,50);
		memcpy(ftp_putPath.remoteFile,destPath,50);
		//上传数据
		res = '0';
		tid = rt_thread_create("ftpput",rt_ftpput_thread_entry, RT_NULL,2048, 14, 20);
		if (tid != RT_NULL) rt_thread_startup(tid);

	}else if(!memcmp(command,"ftpget",6))	//下载数据
	{
		rt_thread_t tid;
		char sourcePath[50]={0x00},destPath[50]={0x00};
		//分割字符串为  命令 [本地源路径] [远程目标路径]
		printf("cmd:%s\n",command);
		splitSpace((char *)command,sourcePath,destPath);
		printf("%s,%s,%s\n","ftpget",sourcePath,destPath);
		memcpy(ftp_getPath.localFile,sourcePath,50);
		memcpy(ftp_getPath.remoteFile,destPath,50);
		//下载数据
		res = '0';
		tid = rt_thread_create("ftpget",rt_ftpget_thread_entry, RT_NULL,2048, 14, 20);
		if (tid != RT_NULL) rt_thread_startup(tid);
	}else if(!memcmp(command,"rm",2))
	{
		char path[50]={0x00};
		memcpy(path,&command[3],(strlen(command)-3));
		path[strlen(command)-3] = '\0';
		printf("cmd:%s %s\n","rm",path);
		unlink(path);
		res = '0';
	}else if(!memcmp(command,"flash",5)) //查看Flash
	{
		long long cap;
		struct statfs buffer;
		dfs_statfs("/", &buffer);
		cap = buffer.f_bsize * buffer.f_bfree / 1024;	//获取到文件系统剩余的大小
		res = responsesFlash(cap);
	}else if(!memcmp(command,"factory",7)) //恢复出厂
	{
		rt_thread_t tid;
		res = '0';	
		tid = rt_thread_create("factory",rt_factory_thread_entry, RT_NULL,2048, 14, 20);
		if (tid != RT_NULL) rt_thread_startup(tid);		 
		
	}else if(!memcmp(command,"network",7)) //查看网络
	{
		//查看当前的网络连接
		res = responsesNetwork(rt_hw_GetWiredNetConnect());
	}
		
	printdecmsg(ECU_DBG_CONTROL_CLIENT,"res",res);

	return res;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(Factory, eg:Factory());
void fact_thread()
{
	rt_thread_t tid;
	tid = rt_thread_create("factory",rt_factory_thread_entry, RT_NULL,2048, 14, 20);
	if (tid != RT_NULL) rt_thread_startup(tid);	
}
FINSH_FUNCTION_EXPORT(fact_thread, eg:fact_thread());

#endif

