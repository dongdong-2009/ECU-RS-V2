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

typedef struct 
{
	char remoteFile[50];
	char localFile[50];
}ftpPath_t;
//��ʱ��
rt_timer_t reboottimer;
rt_timer_t ftpputtimer;
rt_timer_t ftpgettimer;


//��ʱ����ʱ����   ��λ��ʱ
static void reboottimeout(void* parameter)
{
	reboot();
}

void reboot_timer(int timeout)			
{
	reboottimer = rt_timer_create("reboot", /* ��ʱ������Ϊ read */
					reboottimeout, /* ��ʱʱ�ص��Ĵ����� */
					RT_NULL, /* ��ʱ��������ڲ��� */
					timeout*RT_TICK_PER_SECOND, /* ��ʱʱ�䳤��,��OS TickΪ��λ*/
					 RT_TIMER_FLAG_ONE_SHOT); /* �����ڶ�ʱ�� */
	if (reboottimer != RT_NULL) rt_timer_start(reboottimer);
}


ftpPath_t ftp_putPath;
unsigned char ftpput_flag = 0;
//��ʱ����ʱ����  �ϴ����ݳ�ʱ
static void ftpputtimeout(void* parameter)
{
	ftpput_flag = 1;	
}

void ftpput_timer(int timeout)			//zigbee�������ݼ�� ����0 ��ʾ����û������  ����1��ʾ����������
{	
	ftpputtimer = rt_timer_create("ftpput", /* ��ʱ������Ϊ read */
					ftpputtimeout, /* ��ʱʱ�ص��Ĵ����� */
					RT_NULL, /* ��ʱ��������ڲ��� */
					timeout*RT_TICK_PER_SECOND, /* ��ʱʱ�䳤��,��OS TickΪ��λ*/
					 RT_TIMER_FLAG_ONE_SHOT); /* �����ڶ�ʱ�� */
	if (ftpputtimer != RT_NULL) rt_timer_start(ftpputtimer);
}


void rt_ftpput_thread_entry(void* parameter)
{
	ftpput_flag = 0;
	//������ʱ��
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
//��ʱ����ʱ����  �ϴ����ݳ�ʱ
static void ftpgettimeout(void* parameter)
{
	ftpget_flag = 1;	
}

void ftpget_timer(int timeout)			//zigbee�������ݼ�� ����0 ��ʾ����û������  ����1��ʾ����������
{	
	ftpputtimer = rt_timer_create("ftpget", /* ��ʱ������Ϊ read */
					ftpgettimeout, /* ��ʱʱ�ص��Ĵ����� */
					RT_NULL, /* ��ʱ��������ڲ��� */
					timeout*RT_TICK_PER_SECOND, /* ��ʱʱ�䳤��,��OS TickΪ��λ*/
					 RT_TIMER_FLAG_ONE_SHOT); /* �����ڶ�ʱ�� */
	if (ftpputtimer != RT_NULL) rt_timer_start(ftpputtimer);
}


void rt_ftpget_thread_entry(void* parameter)
{
	ftpget_flag = 0;
	//������ʱ��
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


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int mysystem(const char *command)
{
	int res;

	print2msg(ECU_DBG_CONTROL_CLIENT,"Execute:",(char*) command);
	//��Ҫ�������ڴ����
	if(!memcmp(command,"reboot",6))
	{
		res = 0;
		//ͨ����ʱ����λ����
		reboot_timer(10);
	}else if(!memcmp(command,"restart UPDATE",14))
	{
		restartThread(TYPE_UPDATE);
		res = 0;
	}else if(!memcmp(command,"ftpput",6))	//�ϴ�����
	{
		rt_thread_t tid;
		char sourcePath[50],destPath[50];
		//�ָ��ַ���Ϊ  ���� [����Դ·��] [Զ��Ŀ��·��]
		splitSpace((char *)command,sourcePath,destPath);
		printf("cmd:%s\n",command);
		printf("%s,%s,%s\n","ftpput",sourcePath,destPath);
		memcpy(ftp_putPath.localFile,sourcePath,50);
		memcpy(ftp_putPath.remoteFile,destPath,50);
		//�ϴ�����
		res = 0;
		tid = rt_thread_create("ftpput",rt_ftpput_thread_entry, RT_NULL,2048, 14, 20);
		if (tid != RT_NULL) rt_thread_startup(tid);

	}else if(!memcmp(command,"ftpget",6))	//��������
	{
		rt_thread_t tid;
		char sourcePath[50],destPath[50];
		//�ָ��ַ���Ϊ  ���� [����Դ·��] [Զ��Ŀ��·��]
		printf("cmd:%s\n",command);
		splitSpace((char *)command,sourcePath,destPath);
		printf("%s,%s,%s\n","ftpget",sourcePath,destPath);
		memcpy(ftp_getPath.localFile,sourcePath,50);
		memcpy(ftp_getPath.remoteFile,destPath,50);
		//��������
		res = 0;
		tid = rt_thread_create("ftpget",rt_ftpget_thread_entry, RT_NULL,2048, 14, 20);
		if (tid != RT_NULL) rt_thread_startup(tid);
	}else if(!memcmp(command,"rm",2))
	{
		char path[50];
		memcpy(path,&command[3],(strlen(command)-3));
		path[strlen(command)-3] = '\0';
		printf("cmd:%s %s\n","rm",path);
		res = unlink(path);
	}
		
	printdecmsg(ECU_DBG_CONTROL_CLIENT,"res",res);
	if(-1 == res){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: system error.");
		return CMD_ERROR;
	}
	else{

		if(0 == res){
			printmsg(ECU_DBG_CONTROL_CLIENT,"Execute successfully.");
		}
		else{
			printdecmsg(ECU_DBG_CONTROL_CLIENT,"Failed to execute: shell failed", res);
			return CMD_ERROR;
		}

	}
	return SUCCESS;
}
