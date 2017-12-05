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
void clear_zbmodem(void);		//清空串口缓冲区的数据
int openzigbee(void);
void zigbee_reset(void);	//复位zigbee模块
int zb_test_communication(void);		//zigbee测试通信有没有断开
int zb_change_ecu_panid(void);
int zb_off_report_id_and_bind(int short_addr);
int zb_restore_ecu_panid_0xffff(int channel);
int zb_change_inverter_channel_one(char *inverter_id, int channel);
int zigbeeRecvMsg(char *data, int timeout_sec);
int zb_query_heart_data(inverter_info *inverter);
int zb_set_heartSwitch_boardcast(unsigned char functionStatus);
int zb_set_heartSwitch_single(inverter_info *inverter,unsigned char functionStatus);
int zb_sendHeart(char uid[13]);


#endif /*__ZIGBEE_H__*/

