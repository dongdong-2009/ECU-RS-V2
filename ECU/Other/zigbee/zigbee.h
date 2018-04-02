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
void clear_zbmodem(void);		//??????????
int openzigbee(void);
void zigbee_reset(void);	//??zigbee??
int zb_test_communication(void);		//zigbee?????????
int zb_change_ecu_panid(void);
int zb_off_report_id_and_bind(int short_addr);
int zb_restore_ecu_panid_0xffff(int channel);
int zb_change_inverter_channel_one(char *inverter_id, int channel);
int zigbeeRecvMsg(char *data, int timeout_sec);
int zb_query_heart_data(inverter_info *inverter);
int zb_set_heartSwitch_boardcast(unsigned char functionStatus,unsigned char onoff,unsigned char RSDTimeout);
int zb_set_heartSwitch_single(inverter_info *inverter,unsigned char functionStatus,unsigned char onoff,unsigned char RSDTimeout);
int zb_sendHeart(char uid[13]);
int bind_nodata_inverter(inverter_info *firstinverter);
int get_inverter_shortaddress(inverter_info *firstinverter);		//??????????????
int zb_get_reply_update_start(char *data,inverter_info *inverter);			//??????????Update_start???,ZK,????????10?
int zb_get_reply_restore(char *data,inverter_info *inverter);			//???????????,?????????,ZK,?????????,?????????
int zb_send_cmd(inverter_info *inverter, char *buff, int length)	;	//zigbee??
int zb_get_reply(char *data,inverter_info *inverter);			//?????????


#endif /*__ZIGBEE_H__*/

