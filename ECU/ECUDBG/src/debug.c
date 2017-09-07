/*****************************************************************************/
/* File      : debug.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "variation.h"
#include <rtdef.h>
#include <rtthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "string.h"
#include "SEGGER_RTT.h"
#include "rtc.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
char funname[9][20] = {
		"update",
		"comm",
		"event",
		"collect",
		"client",
		"control",
		"file",
		"wifi",
		"other"
};

char time[15];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

void printmsg(DebugType type,char *msg)		//打印字符串
{
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG	
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s!\n",time,funname[type], msg);
	SEGGER_RTT_printf(0,"%s",string);
	free(string);
#endif
	
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s!\n",time,funname[type], msg);
			#endif
			break;
	}

#endif
}

void print2msg(DebugType type,char *msg1, char *msg2)		//打印字符串
{
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
	SEGGER_RTT_printf(0,"%s",string);
	free(string);
#endif

#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s: %s!\n",time,funname[type], msg1, msg2);
			#endif
			break;
	}
	
#endif
}

void printdecmsg(DebugType type,char *msg, int data)		//打印整形数据
{
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s: %d!\n",time,funname[type], msg, data);
	SEGGER_RTT_printf(0,"%s",string);
	free(string);
#endif

#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
	case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s: %d!\n",time,funname[type], msg, data);
			#endif
			break;
	}	
#endif
}

void printhexdatamsg(DebugType type,char *msg, int data)		//打印16进制数据,ZK
{
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s: %X!\n",time,funname[type], msg, data);
	SEGGER_RTT_printf(0,"%s",string);
	free(string);
#endif

#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s: %X!\n",time,funname[type], msg, data);
			#endif
			break;
	}
#endif
}

void printfloatmsg(DebugType type,char *msg, float data)		//打印实数
{
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s: %f!\n",time,funname[type], msg, data);
	SEGGER_RTT_printf(0,"%s",string);
	free(string);
#endif

#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s: %f!\n",time,funname[type], msg, data);
			#endif
			break;
	}	
	
#endif
}

void printhexmsg(DebugType type,char *msg, char *data, int size)		//打印十六进制数据
{
	int i;
	memset(time,'\0',15);
	apstime(time);
	
#if ECU_JLINK_DEBUG
	char *string = NULL;
	string = malloc(1024);
	sprintf(string,"%s:%s==>%s: ",time,funname[type], msg);
	SEGGER_RTT_printf(0,"%s",string);
	for(i=0; i<size; i++)
			SEGGER_RTT_printf(0,"%02X, ", data[i]);
	SEGGER_RTT_printf(0,"\n");
	free(string);
#endif	
#if ECU_DEBUG
	
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_COMM:
			#if ECU_DEBUG_COMM
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_EVENT:
			#if ECU_DEBUG_EVENT
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_COLLECT:
			#if ECU_DEBUG_COLLECT
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG__CONTROL_CLIENT
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_FILE:
			#if ECU_DEBUG_FILE
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_WIFI:
			#if ECU_DEBUG_WIFI
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s:%s==>%s: ",time,funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
	}
#endif
}


