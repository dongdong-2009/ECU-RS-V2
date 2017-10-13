#ifndef __CHANNEL_H__ 
#define	__CHANNEL_H__
/*****************************************************************************/
/*  File      : channel.h                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int process_channel(void);
int channel_need_change(void);
int saveOldChannel(unsigned char oldChannel);
int saveNewChannel(unsigned char newChannel);
int saveChannel_change_flag(void);
int getOldChannel(void);
int getNewChannel(void);
int saveECUChannel(int channel);
void changeChannelOfInverters(int oldChannel, int newChannel);

#endif /*__CHANNEL_H__*/
