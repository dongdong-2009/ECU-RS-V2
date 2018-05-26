#ifndef __ZIGBEE_CHANNEL_H__
#define __ZIGBEE_CHANNEL_H__
/*****************************************************************************/
/* File      : ZigBeeChannel.h                                               */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-05-25 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int set_ZigBeeChannel(const char *recvbuffer, char *sendbuffer);
int response_ZigBeeChannel_Result(const char *recvbuffer, char *sendbuffer);

void ResponseZigbeeChannel(char *UID,char channel,unsigned short panid,unsigned short shortadd);
void ResponseECUZigbeeChannel(char channel,unsigned short panid,unsigned short shortadd);

#endif	/*__ZIGBEE_CHANNEL_H__*/
