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
#include "arch/sys_arch.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/


/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
//�����յ�������
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id);
//01 �����Ӧ
void APP_Response_BaseInfo(unsigned char *ID,char *ECU_NO,char *TYPE,char SIGNAL_LEVEL,char *SIGNAL_CHANNEL,int Length,char * Version,inverter_info *inverter,int validNum);
//02 �����Ӧ
void APP_Response_SystemInfo(unsigned char *ID,unsigned char mapflag,inverter_info *inverter,int validNum);
//03 �����Ӧ
void APP_Response_SetNetwork(unsigned char *ID,unsigned char result);
//04 �����Ӧ
void APP_Response_SetChannel(unsigned char *ID,unsigned char mapflag,char *SIGNAL_CHANNEL,char SIGNAL_LEVEL);
//05 �����Ӧ
void APP_Response_SetWifiPassword(unsigned char *ID,unsigned char result);
//06 �����Ӧ
void APP_Response_IOInitStatus(unsigned char *ID,unsigned char result);
//07 �����Ӧ
void APP_Response_GetRSDHistoryInfo(char mapping,unsigned char *ID,char *date_time ,char * UID);
//08 �����Ӧ
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char *date_time ,char request_type);
//09 �����Ӧ
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID);
//10 �����Ӧ
void APP_Response_GetWiredNetwork(char mapping,unsigned char *ID,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr,char *MacAddress);
//11 �����Ӧ
void APP_Response_FlashSize(char mapping,unsigned char *ID,unsigned int Flashsize);
//12 �����Ӧ
void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date);

#endif /*__APPCOMM_H__*/
