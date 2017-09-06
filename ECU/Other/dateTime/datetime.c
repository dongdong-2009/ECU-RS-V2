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
int get_time(char *sendcommanddatetime, char *sendcommandtime)		//发给EMA记录时获取的时间，格式：年月日时分秒，如20120902142835
{
	char datetime[15] = {'\0'};
	unsigned hour, minute;
	apstime(datetime);
	rt_memcpy(sendcommanddatetime,datetime,14);
	sendcommanddatetime[14] = '\0';
	hour = ((datetime[8] - 0x30) * 10) + (datetime[9] - 0x30);
	minute = ((datetime[10] - 0x30) * 10) + (datetime[11] - 0x30);
    
	sendcommandtime[0] = hour;
	sendcommandtime[1] = minute;
    
	return hour;
}


//获取日期，用于当天发电量，格式：年月日，如20120902
void getdate(char date[10])		
{
	char datetime[15] = {'\0'};
	apstime(datetime);
	rt_memcpy(date,datetime,8);
	

}

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

//<
// >=
int compareTimeLess(int durabletime ,int thistime,int reportinterval)
{
	if((durabletime < reportinterval) && (thistime > reportinterval))
	{
		if((durabletime+(24*60*60+1)-thistime) <= reportinterval)
		{
			return 1;
		}
		
	}else
	{
		if((durabletime-thistime) < reportinterval)
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

void getcurrenttime(char db_time[])		
{
	apstime(db_time);
	db_time[14] = '\0';
}
