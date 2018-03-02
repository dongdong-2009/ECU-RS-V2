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
#include "variation.h"

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
void APP_Response_BaseInfo(ecu_info curecu,int Length,char * Version,inverter_info *inverter);
//02 �����Ӧ
void APP_Response_SystemInfo(unsigned char mapflag,inverter_info *inverter,int validNum);
//03 �����Ӧ
void APP_Response_PowerCurve(char mapping,char * date);
//04 �����Ӧ
void APP_Response_GenerationCurve(char mapping,char request_type);
//05 �����Ӧ
void APP_Response_SetNetwork(unsigned char result);
//06 �����Ӧ
void APP_Response_SetTime(char mapping);
//07 �����Ӧ
void APP_Response_SetWiredNetwork(char mapping);
//08 �����Ӧ
void APP_Response_GetECUHardwareStatus(unsigned char mapping);
//10 �����Ӧ
void APP_Response_SetWifiPassword(unsigned char result);
//11 �����Ӧ
void APP_Response_GetIDInfo(char mapping,inverter_info *inverter);
//12 �����Ӧ
void APP_Response_GetTime(char mapping,char *Time);
//13 �����Ӧ
void APP_Response_FlashSize(char mapping,unsigned int Flashsize);
//14 �����Ӧ
void APP_Response_GetWiredNetwork(char mapping,char dhcpStatus,IP_t IPAddr,IP_t MSKAddr,IP_t GWAddr,IP_t DNS1Addr,IP_t DNS2Addr,char *MacAddress);
//15 �����Ӧ
void APP_Response_SetChannel(unsigned char mapflag,char SIGNAL_CHANNEL,char SIGNAL_LEVEL);
//16 �����Ӧ
void APP_Response_IOInitStatus(unsigned char result);
//17 �����Ӧ
void APP_Response_GetRSDHistoryInfo(char mapping,char *date_time ,char * UID);
//18 �����Ӧ
void APP_Response_GetShortAddrInfo(char mapping,inverter_info *inverter);
//20 �����Ӧ
void APP_Response_GetECUAPInfo(char mapping,unsigned char connectStatus,char *info);
//21 �����Ӧ
void APP_Response_SetECUAPInfo(unsigned char result);
//22 �����Ӧ
void APP_Response_GetECUAPList(char mapping,char *list);
//23 �����Ӧ
void APP_Response_GetFunctionStatusInfo(char mapping);

#endif /*__APPCOMM_H__*/
