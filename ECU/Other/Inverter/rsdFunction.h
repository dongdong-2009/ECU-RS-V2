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
int process_rsdFunction(void);
int rsdFunction_need_change(void);
int saveChangeFunctionStatus(unsigned char FunctionStatus);
int save_rsdFunction_change_flag(void);
int getChangeFunctionStatus(void);
void changeRSDFunctionOfInverters(unsigned char curFunctionStatus, unsigned char changeFunctionStatus);

#endif /*__RSD_FUNCTION_H__*/
