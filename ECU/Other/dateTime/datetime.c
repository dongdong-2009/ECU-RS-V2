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
#include <stdio.h>

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

// 时间结构体 
typedef struct{  
       unsigned char second;  
       unsigned char minute;  
       unsigned char hour;  
       unsigned char day;    
       unsigned char month;  
       unsigned char year;  
       unsigned char century;  
}DATETIME;  

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
    
	//print2msg(ECU_DBG_OTHER,"Broadcast time", sendcommanddatetime);


	return hour;
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



int get_hour()
{
	char datetime[15] = {'\0'};
	unsigned hour;
	apstime(datetime);
	hour = ((datetime[8] - 0x30) * 10) + (datetime[9] - 0x30);
	return hour;
}

//时间转换 Time为当前的时间字符串  14字节
DATETIME timeSwitch(char *Time)
{
	DATETIME tmpdatetime;
	tmpdatetime.century = (Time[0]-'0')*10+(Time[1]-'0');
	
	tmpdatetime.year = (Time[2]-'0')*10+(Time[3]-'0');
	tmpdatetime.month = (Time[4]-'0')*10+(Time[5]-'0');
	tmpdatetime.day = (Time[6]-'0')*10+(Time[7]-'0');
	tmpdatetime.hour = (Time[8]-'0')*10+(Time[9]-'0');
	tmpdatetime.minute = (Time[10]-'0')*10+(Time[11]-'0');
	tmpdatetime.second = (Time[12]-'0')*10+(Time[13]-'0');
	return tmpdatetime;
}

/******************************************************************* 
 *  函数功能:将当前时间(如:2017-09-7 21:28:25)转换为格林尼治时间  格林尼治时间:从1970年1月1日开始到现在的秒数 
 *  函数输入:curData 当前时间 
 *  函数输出:格林尼治时间(单位:S) 
 */  
long GreenSwitch( DATETIME curData )  
{  
       unsigned short curyear;         // 当前年份 16位  
       int cyear = 0;                  // 当前年和1970年的差值 
       int cday = 0;                   // 差值年转换为天数  
       int curmonthday = 0;            // 当前月份转换为天数
   
       //当前年份  
       curyear= curData.century*100+curData.year;  
       //无效年份判断 
       if(curyear < 1970 || curyear > 9000 )  
              return 0;  
       //计算差值并计算当前年份差值对应的天数 
       cyear= curyear - 1970;  
       cday= cyear/4*(365*3+366);  
       //计算平年和闰年 对应的相应差值年对应的天数 1970-平 1971-平 1972-闰 1973-平  
       if(cyear%4 >= 3 )  
              cday += (cyear%4-1)*365 + 366;  
       else  
              cday += (cyear%4)*365;  
   
       //当前月份对应的天数  当月的天数不算  
       switch(curData.month )  
       {  
              case 2:  curmonthday = 31;  break;  
              case 3:  curmonthday = 59;  break;  
              case 4:  curmonthday = 90;  break;  
              case 5:  curmonthday = 120; break;  
              case 6:  curmonthday = 151; break;  
              case 7:  curmonthday = 181; break;  
              case 8:  curmonthday = 212; break;  
              case 9:  curmonthday = 243; break;  
              case 10: curmonthday = 273; break;  
              case 11: curmonthday = 304; break;  
              case 12: curmonthday = 334; break;  
              default:curmonthday = 0;   break;  
       }  
       //平年和闰年对应天数 如果闰年+1  
       if((curyear%4 == 0) && (curData.month >= 3) )  
              curmonthday+= 1;  
       //总天数加上月份对应的天数 加上当前天数-1 当前天数不算  
       cday += curmonthday;  
       cday += (curData.day-1);  
   
       //返回格林尼治时间秒数 
       return(long)(((cday*24+curData.hour)*60+curData.minute)*60+curData.second);   
}  



int Time_difference(char *curTime,char *lastTime)
{
	DATETIME curDateTime,lastDateTime;
	
	curDateTime = timeSwitch(curTime);

	lastDateTime = timeSwitch(lastTime);
	
	return (GreenSwitch(curDateTime) - GreenSwitch(lastDateTime));
}

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <stdio.h>
void greenTime(char *time)
{
	DATETIME curDateTime;
	curDateTime = timeSwitch(time);
	printf("Greentime:%ld\n",GreenSwitch(curDateTime));
}
FINSH_FUNCTION_EXPORT(greenTime, get green time. e.g: greenTime("20170908084018"))


#endif
