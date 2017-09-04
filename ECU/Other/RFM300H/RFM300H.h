/*****************************************************************************/
/* File      : rfm300h.h                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-04 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __RFM300H_H__
#define __RFM300H_H__

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "CMT2300.h"
#include "variation.h"

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int RFM300_Bind_Uid(char *ECUID,char *UID,char channel,char rate,char *ver);
int RFM300_Heart_Beat(char *ECUID,inverter_info * cur_inverter);
int RFM300_Status_Init(char *ECUID,char *UID,char Heart_Function,char Device_Type,status_t *status);
int RFM300_Set_Uid(char *ECUID,char *UID,int channel,int rate,char *NewUid,char *SaveChannel,char *SaveRate);
#endif /*__RFM300H_H__*/

