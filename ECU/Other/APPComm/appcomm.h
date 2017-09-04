/*****************************************************************************/
/* File      : appcomm.h                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-08 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __APPCOMM_H__
#define __APPCOMM_H__
/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "usart.h"
#include "variation.h"
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

typedef enum
{ 
    COMMAND_BASEINFO         	= 1,		//接收数据头
    COMMAND_SYSTEMINFO       	= 2,	//接收数据长度   其中数据部分的长度为接收到长度减去12个字节
    COMMAND_SETNETWORK      	= 3,	//接收数据部分数据
    COMMAND_SETCHANNEL       	= 4,		//接收END结尾标志
		COMMAND_SETWIFIPASSWORD 	= 5,
		COMMAND_IOINITSTATUS 			= 6,
} eCommandID;// receive state machin

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//解析收到的数据
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id);
//01 命令回应
void APP_Response_BaseInfo(unsigned char *ID,char *ECU_NO,char *TYPE,char SIGNAL_LEVEL,char *SIGNAL_CHANNEL,int Length,char * Version,inverter_info *inverter,int validNum);
//02 命令回应
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int validNum);
//03 命令回应
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result);
//04 命令回应
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char *SIGNAL_CHANNEL,char SIGNAL_LEVEL);
//05 命令回应
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result);
//06 命令回应
void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result);
#endif /*__APPCOMM_H__*/
