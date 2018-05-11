/*****************************************************************************/
/* File      : event.h                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-09 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#ifndef __EVENT_H__
#define __EVENT_H__
#include "variation.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
void add_APP_functions(void);	//����ֻ�APP��Ӧ������
int setECUID(char *ECUID);
void process_HeartBeatEvent(void);		//��������ʱ��
void process_WIFI(void);	//����WIFI�¼�
void process_KEYEvent(void);		//�������¼�
int process_WIFI_RST(void);			//����λʱ��
void process_WIFIEvent_ESP07S(void);
void WIFI_GetEvent_ESP07S(void);
void process_APKEYEvent(void);
void process_switchSTAMode(void);
#endif /*__EVENT_H__*/

