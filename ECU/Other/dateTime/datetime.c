/*****************************************************************************/
/*  File      : datatime.c                                                   */
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
#include "rtc.h"
#include "datetime.h"
#include <string.h>
#include <rtthread.h>

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/


int acquire_time()
{
	char datetime[15] = {'\0'};
	unsigned char hour, minute, second;
	apstime(datetime);
	hour = ((datetime[8] - 0x30) *10) + (datetime[9] - 0x30);
	minute = ((datetime[10] - 0x30) *10) + (datetime[11] - 0x30);
	second = ((datetime[12] - 0x30) *10) + (datetime[13] - 0x30);
	//rt_kprintf("-*******---   %d %d %d \n",hour,minute,second);
	return (hour*60*60+minute*60+second);
}

// >=
int compareTime(int durabletime ,int thistime,int reportinterval)
{
	if((durabletime < reportinterval) && (thistime > reportinterval))
	{
		if((durabletime+(24*60*60+1)-thistime) > reportinterval)
		{
			return 1;
		}
		
	}else
	{
		if((durabletime-thistime) >= reportinterval)
		{
			return 1;
		}
	}
	return 0;
}



int get_hour()
{
	char datetime[15] = {'\0'};
	unsigned hour;
	apstime(datetime);
	hour = ((datetime[8] - 0x30) * 10) + (datetime[9] - 0x30);
	return hour;
}


int Time_difference(char *curTime,char *lastTime)
{
	unsigned char curhour, curminute, cursecond;
	unsigned char lasthour, lastminute, lastsecond;
	curhour = ((curTime[8] - 0x30) *10) + (curTime[9] - 0x30);
	curminute = ((curTime[10] - 0x30) *10) + (curTime[11] - 0x30);
	cursecond = ((curTime[12] - 0x30) *10) + (curTime[13] - 0x30);
	
	lasthour = ((lastTime[8] - 0x30) *10) + (lastTime[9] - 0x30);
	lastminute = ((lastTime[10] - 0x30) *10) + (lastTime[11] - 0x30);
	lastsecond = ((lastTime[12] - 0x30) *10) + (lastTime[13] - 0x30);
	
	
}
