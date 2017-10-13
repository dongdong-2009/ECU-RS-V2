#ifndef __BIND_INVERTERS_H__
#define __BIND_INVERTERS_H__
/*****************************************************************************/
/*  File      : bind_inverters.h                                             */
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
void bind_inverters(void);
void setPanidOfInverters(int num, char **ids);
int getShortaddrOfInverters(int num, char **ids);
int getShortaddrOfEachInverter(char *id);

#endif /*__BIND_INVERTERS_H__*/
