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
    COMMAND_BASEINFO         	= 1,		//��������ͷ
    COMMAND_SYSTEMINFO       	= 2,	//�������ݳ���   �������ݲ��ֵĳ���Ϊ���յ����ȼ�ȥ12���ֽ�
    COMMAND_SETNETWORK      	= 3,	//�������ݲ�������
    COMMAND_SETCHANNEL       	= 4,		//����END��β��־
		COMMAND_SETWIFIPASSWORD 	= 5,
		COMMAND_IOINITSTATUS 			= 6,
} eCommandID;// receive state machin

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
#endif /*__APPCOMM_H__*/
