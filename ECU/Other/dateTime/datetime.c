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

// ʱ��ṹ�� 
typedef struct{  
       unsigned char second;  
       unsigned char minute;  
       unsigned char hour;  
       unsigned char day;    
       unsigned char month;  
       unsigned char year;  
       unsigned char century;  
}DATETIME;  

int get_time(char *sendcommanddatetime, char *sendcommandtime)		//����EMA��¼ʱ��ȡ��ʱ�䣬��ʽ��������ʱ���룬��20120902142835
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

//ʱ��ת�� TimeΪ��ǰ��ʱ���ַ���  14�ֽ�
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
 *  ��������:����ǰʱ��(��:2017-09-7 21:28:25)ת��Ϊ��������ʱ��  ��������ʱ��:��1970��1��1�տ�ʼ�����ڵ����� 
 *  ��������:curData ��ǰʱ�� 
 *  �������:��������ʱ��(��λ:S) 
 */  
long GreenSwitch( DATETIME curData )  
{  
       unsigned short curyear;         // ��ǰ��� 16λ  
       int cyear = 0;                  // ��ǰ���1970��Ĳ�ֵ 
       int cday = 0;                   // ��ֵ��ת��Ϊ����  
       int curmonthday = 0;            // ��ǰ�·�ת��Ϊ����
   
       //��ǰ���  
       curyear= curData.century*100+curData.year;  
       //��Ч����ж� 
       if(curyear < 1970 || curyear > 9000 )  
              return 0;  
       //�����ֵ�����㵱ǰ��ݲ�ֵ��Ӧ������ 
       cyear= curyear - 1970;  
       cday= cyear/4*(365*3+366);  
       //����ƽ������� ��Ӧ����Ӧ��ֵ���Ӧ������ 1970-ƽ 1971-ƽ 1972-�� 1973-ƽ  
       if(cyear%4 >= 3 )  
              cday += (cyear%4-1)*365 + 366;  
       else  
              cday += (cyear%4)*365;  
   
       //��ǰ�·ݶ�Ӧ������  ���µ���������  
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
       //ƽ��������Ӧ���� �������+1  
       if((curyear%4 == 0) && (curData.month >= 3) )  
              curmonthday+= 1;  
       //�����������·ݶ�Ӧ������ ���ϵ�ǰ����-1 ��ǰ��������  
       cday += curmonthday;  
       cday += (curData.day-1);  
   
       //���ظ�������ʱ������ 
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
