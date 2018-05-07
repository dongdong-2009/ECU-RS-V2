/*****************************************************************************/
/* File      : third_inverter.h                                              */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-04-24 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __THIRD_INVERTER_H__
#define __THIRD_INVERTER_H__
/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "variation.h"


/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
void updateThirdID(void);
int get_ThirdID_from_file(inverter_third_info *firstThirdinverter);
void init_Third_Inverter(inverter_third_info *inverter);

void getAllThirdInverterData(void);
void Debug_ThirdInverter_info(inverter_third_info *curThirdinverter);
unsigned int get_ThirdBaudRate(char cBaudrate);


#endif /*__THIRD_INVERTER_H__*/
