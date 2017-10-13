#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__
/*****************************************************************************/
/*  File      : zigbee.h                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "variation.h"

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
void clear_zbmodem(void);		//��մ��ڻ�����������
int openzigbee(void);
void zigbee_reset(void);	//��λzigbeeģ��
int zb_test_communication(void);		//zigbee����ͨ����û�жϿ�
int zb_change_ecu_panid(void);
int zb_off_report_id_and_bind(int short_addr);
int zb_restore_ecu_panid_0xffff(int channel);
int zb_change_inverter_channel_one(char *inverter_id, int channel);
int zigbeeRecvMsg(char *data, int timeout_sec);
int zb_query_heart_data(inverter_info *inverter);

#endif /*__ZIGBEE_H__*/

