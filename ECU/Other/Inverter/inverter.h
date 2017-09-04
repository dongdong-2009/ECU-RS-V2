/*****************************************************************************/
/* File      : inverter.h                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __INVERTER_H__
#define __INVERTER_H__

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "variation.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern char ECUID12[13];
extern char ECUID6[7];
extern char Signal_Level;
extern char Signal_Channel[3];
extern char Channel_char;
extern char IO_Init_Status;			//IO��ʼ״̬
extern char ver;						//�Ż����汾��
extern int validNum;				//��ǰ��Ч̨��
extern int curSequence;		//������ѵ������
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern int Data_Len;
extern int Command_Id;
extern int ResolveFlag ;
extern int messageLen;
extern int messageUsart1Len;
extern int UART1_Data_Len;
extern int UART1_Command_Id;
extern int UART1_ResolveFlag;

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int init_ecu(void);
int init_inverter(inverter_info *inverter);
int add_inverter(inverter_info *inverter,int num,char *uidstring);
#endif /*__INVERTER_H__*/
