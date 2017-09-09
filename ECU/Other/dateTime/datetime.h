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
//ʱ��Ƚ�
int compareTime(int durabletime ,int thistime,int reportinterval);
//��ȡ��ǰ��Сʱ��
int get_hour(void);

//��������ʱ����ʱ���
int Time_difference(char *curTime,char *lastTime);

#endif /*__DATETIME_H__*/
