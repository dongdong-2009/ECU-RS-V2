#ifndef __DATETIME_H__
#define __DATETIME_H__
/*****************************************************************************/
/*  File      : datatime.h                                                   */
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

int get_time(char *sendcommanddatetime, char *sendcommandtime);
int acquire_time(void);
//时间比较
int compareTime(int durabletime ,int thistime,int reportinterval);
//获取当前的小时数
int get_hour(void);

//计算两个时间点的时间差
int Time_difference(char *curTime,char *lastTime);

#endif /*__DATETIME_H__*/
