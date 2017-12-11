#ifndef __RSD_FUNCTION_H__
#define __RSD_FUNCTION_H__
/*****************************************************************************/
/*  File      : rsdFunction.h                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-10-16 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int process_rsdFunction_all(void);
int process_rsdFunction(void);
void insertSetRSDInfo(unsigned short num,char *buff);
int rsdFunction_need_change(void);
int saveChangeFunctionStatus(unsigned char FunctionStatus,unsigned char onoffstatus,unsigned char rsdTimeout);
int save_rsdFunction_change_flag(void);
int getChangeFunctionStatus(unsigned char *FunctionStatus,unsigned char *onoffstatus,unsigned char *rsdTimeout);
void changeRSDFunctionOfInverters(unsigned char changeFunctionStatus,unsigned char onoffStatus,unsigned char rsdTimeout);

#endif /*__RSD_FUNCTION_H__*/
