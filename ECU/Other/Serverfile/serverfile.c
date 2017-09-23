#include "Serverfile.h"
#include <dfs_posix.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datetime.h"
#include "SEGGER_RTT.h"
#include "dfs_fs.h"
#include "rthw.h"
#include "variation.h"
#include "debug.h"
#include "string.h"
#include "rtc.h"


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
rt_mutex_t record_data_lock = RT_NULL;
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
#define EPSILON 0.000000001
int day_tab[2][12]={{31,28,31,30,31,30,31,31,30,31,30,31},{31,29,31,30,31,30,31,31,30,31,30,31}}; 


int leap(int year) 
{ 
    if(year%4==0 && year%100!=0 || year%400==0) 
        return 1; 
    else 
        return 0; 
} 

int fileopen(const char *file, int flags, int mode)
{
	return open(file,flags,mode);
}

int fileclose(int fd)
{
	return close(fd);
}

int fileWrite(int fd,char* buf,int len)
{
	return write( fd, buf, len );
}

int fileRead(int fd,char* buf,int len)
{
	return read( fd, buf, len );
}


//��CSV�ļ��е�һ��ת��Ϊ�ַ���
int splitString(char *data,char splitdata[][32])
{
	int i,j = 0,k=0;

	for(i=0;i<strlen(data);++i){
		
		if(data[i] == ',') {
			splitdata[j][k] = 0;
			++j;
			k = 0; 
		}
		else{
			splitdata[j][k] = data[i];
			++k;
		}
	}
	return j+1;

}

//�ָ�ո�
void splitSpace(char *data,char *sourcePath,char *destPath)
{
	int i,j = 0,k = 0;
	char splitdata[3][50];
	for(i=0;i<strlen(data);++i){
		if(data[i] == ' ') {
			splitdata[j][k] = 0;
			++j;
			k = 0; 
		}
		else{
			splitdata[j][k] = data[i];
			++k;
		}
	}

	memcpy(sourcePath,splitdata[1],strlen(splitdata[1]));
	sourcePath[strlen(splitdata[1])] = '\0';
	memcpy(destPath,splitdata[2],strlen(splitdata[2]));
	destPath[strlen(splitdata[2])-2] = '\0';
}

//��ʼ��
void init_tmpdb(inverter_info *firstinverter)
{
	int j;
	char list[4][32];
	char data[200];
	unsigned char UID6[7] = {'\0'};
	FILE *fp;
	inverter_info *curinverter = firstinverter;
	fp = fopen("/home/data/collect.con", "r");
	if(fp)
	{
		while(NULL != fgets(data,200,fp))
		{
			//print2msg(ECU_DBG_FILE,"ID",data);
			memset(list,0,sizeof(list));
			splitString(data,list);
			//�ж��Ƿ���ڸ������
			//��12λ��RSD IDת��Ϊ6λ��BCD����ID
			UID6[0] = (list[0][0]-'0')*0x10 + (list[0][1]-'0');
			UID6[1] = (list[0][2]-'0')*0x10 + (list[0][3]-'0');
			UID6[2] = (list[0][4]-'0')*0x10 + (list[0][5]-'0');
			UID6[3] = (list[0][6]-'0')*0x10 + (list[0][7]-'0');
			UID6[4] = (list[0][8]-'0')*0x10 + (list[0][9]-'0');
			UID6[5] = (list[0][10]-'0')*0x10 + (list[0][11]-'0');
			UID6[6] = '\0';
			curinverter = firstinverter;
			for(j=0; (j<ecu.validNum); j++)	
			{

				if(!memcmp(curinverter->uid,UID6,2))
				{
					memcpy(curinverter->LastCollectTime,list[1],14);
					curinverter->LastCollectTime[14] = '\0';
					curinverter->Last_PV1_Energy = atoi(list[2]);
					curinverter->Last_PV2_Energy = atoi(list[3]);
					printf("UID %s ,LastCollectTime: %s ,Last_PV1_Energy: %d ,Last_PV2_Energy: %d \n",list[0],curinverter->LastCollectTime,curinverter->Last_PV1_Energy,curinverter->Last_PV2_Energy);
					break;
				}
				curinverter++;
			}			
		}
		printf("\n\n");
		fclose(fp);
	}
}

//��ʼ��Record��
void init_RecordMutex(void)
{
	record_data_lock = rt_mutex_create("record_data_lock", RT_IPC_FLAG_FIFO);
	if (record_data_lock != RT_NULL)
	{

		rt_kprintf("Initialize record_data_lock successful!\n");
	}
}

//�����ַ����ļ�
void echo(const char* filename,const char* string)
{
	int fd;
	int length;
	if((filename == NULL) ||(string == NULL))
	{
		printmsg(ECU_DBG_FILE,"para error");
		return ;
	}

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		printmsg(ECU_DBG_FILE,"open file for write failed");
		return;
	}
	length = write(fd, string, strlen(string));
	if (length != strlen(string))
	{
		printmsg(ECU_DBG_FILE,"check: read file failed");
		close(fd);
		return;
	}
	close(fd);
}


int initPath(void)
{
	mkdir("/home",0x777);
	rt_hw_ms_delay(20);
	mkdir("/config",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record",0x777);
	rt_hw_ms_delay(20);
	echo("/home/data/ltpower","0.000000");
	rt_hw_ms_delay(20);
	mkdir("/home/record/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/power",0x777);
	mkdir("/home/record/energy",0x777);
	echo("/config/ftpadd.con", "IP=60.190.131.190\nPort=9219\nuser=zhyf\npassword=yuneng\n");
	rt_hw_ms_delay(20);
	mkdir("/ftp",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/ctldata/",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/almdata/",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/rsdinfo/",0x777);	
	
	return 0;
}

//��ʼ���ļ�ϵͳ
int initsystem(char *mac)
{
	initPath();
	rt_hw_ms_delay(20);
	echo("/config/ecumac.con",mac);
	
	return 0;
}

//�������ֽڵ��ַ���ת��Ϊ16����
int strtohex(char str[2])
{
	int ret=0;
	
	((str[0]-'A') >= 0)? (ret =ret+(str[0]-'A'+10)*16 ):(ret = ret+(str[0]-'0')*16);
	((str[1]-'A') >= 0)? (ret =ret+(str[1]-'A'+10) ):(ret = ret+(str[1]-'0'));
	return ret;
}

void get_mac(rt_uint8_t  dev_addr[6])
{
	FILE *fp;
	char macstr[18] = {'\0'};
	fp = fopen("/config/ecumac.con","r");
	if(fp)
	{
		//��ȡmac��ַ
		if(NULL != fgets(macstr,18,fp))
		{
			dev_addr[0]=strtohex(&macstr[0]);
			dev_addr[1]=strtohex(&macstr[3]);
			dev_addr[2]=strtohex(&macstr[6]);
			dev_addr[3]=strtohex(&macstr[9]);
			dev_addr[4]=strtohex(&macstr[12]);
			dev_addr[5]=strtohex(&macstr[15]);
			fclose(fp);
			return;
		}
		fclose(fp);		
	}
	dev_addr[0]=0x00;;
	dev_addr[1]=0x80;;
	dev_addr[2]=0xE1;
	dev_addr[3]=*(rt_uint8_t*)(0x1FFFF7E8+7);
	dev_addr[4]=*(rt_uint8_t*)(0x1FFFF7E8+8);
	dev_addr[5]=*(rt_uint8_t*)(0x1FFFF7E8+9);
	return;
}


int delete_line(char* filename,char* temfilename,char* compareData,int len)
{
	FILE *fin,*ftp;
  char data[512] = {'\0'};
  fin=fopen(filename,"r");//��ֻ����ʽ���ļ�1
	if(fin == NULL)
	{
		print2msg(ECU_DBG_OTHER,"Open the file failure",filename);
    return -1;
	}
	
  ftp=fopen(temfilename,"w");
	if( ftp==NULL){
		print2msg(ECU_DBG_OTHER,"Open the filefailure",temfilename);
		fclose(fin);
    return -1;
  }
  while(fgets(data,512,fin))//��ȡ������
	{
		if(memcmp(data,compareData,len))
		{
			//print2msg(ECU_DBG_OTHER,"delete_line",data);
			fputs(data,ftp);//������ϱ�׼�����ݵ��ļ�1
		}
	}
  fclose(fin);
  fclose(ftp);
  remove(filename);//ɾ��ԭ�����ļ�
  rename(temfilename,filename);//����ʱ�ļ�����Ϊԭ�����ļ�
  return 0;
	
}

//��ѯ�Ƿ�����������
int search_line(char* filename,char* compareData,int len)
{
	FILE *fin;
  char data[300];
  fin=fopen(filename,"r");
	if(fin == NULL)
	{
		print2msg(ECU_DBG_OTHER,"search_line failure1",filename);
    return -1;
	}
	
  while(fgets(data,300,fin))//��ȡ300���ֽ�
	{
		if(!memcmp(data,compareData,len))
		{
			//��ȡ���˹ر��ļ�������1 ��ʾ���ڸ������ݡ�
			fclose(fin);
			return 1;
		}
	}
  fclose(fin);
  return -1;
}


//������
int insert_line(char * filename,char *str)
{
	int fd;
	fd = open(filename, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		

		write(fd,str,strlen(str));
		close(fd);
	}
	
	return search_line(filename,str,strlen(str));
	
}

//���Ŀ¼��ʱ��������ļ�,���ڷ���1�������ڷ���0
//dirΪĿ¼�����ƣ����������    oldfileΪ�����ļ����ļ���������������
static int checkOldFile(char *dir,char *oldFile)
{
	DIR *dirp;
	struct dirent *d;
	char path[100] , fullpath[100] = {'\0'};
	int fileDate = 0,temp = 0;
	char tempDate[9] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir(dir);
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_OTHER,"check Old File open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				
				memcpy(tempDate,d->d_name,8);
				tempDate[8] = '\0';
				if(((temp = atoi(tempDate)) < fileDate) || (fileDate == 0))
				{
					fileDate = temp;
					memset(path,0,100);
					strcpy(path,d->d_name);
				}
				
			}
			if(fileDate != 0)
			{
				sprintf(fullpath,"%s/%s",dir,path);
				strcpy(oldFile,fullpath);
				closedir(dirp);
				return 1;
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}


//���������ļ�ϵͳ���ж�ʣ��ռ�洢�������ʣ��ɴ洢�ռ��С���������Ӧ��Ŀ¼����������Ӧ��ɾ������
int optimizeFileSystem(void)
{
  int result;
  long long cap;
  struct statfs buffer;
	char oldFile[100] = {'\0'};

  result = dfs_statfs("/", &buffer);
  if (result != 0)
  {
      printmsg(ECU_DBG_OTHER,"dfs_statfs failed.\n");
      return -1;
  }
  cap = buffer.f_bsize * buffer.f_bfree / 1024;
	
	printdecmsg(ECU_DBG_FILE,"disk free size",(unsigned long)cap);
	//��flashоƬ��ʣ�µ�����С��40KB��ʱ�����һЩ��Ҫ���ļ�ɾ��������
	if (cap < 40) 
	{
		//ɾ����ǰ��һ���ECU������������    �����Ŀ¼�´����ļ��Ļ�
		if(1 == checkOldFile("/home/record/almdata",oldFile))
		{
			unlink(oldFile);
		}
		
		//ɾ����ǰ��һ��������������������  �����Ŀ¼�´����ļ��Ļ�
		memset(oldFile,0x00,100);
		if(1 == checkOldFile("/home/record/ctldata",oldFile))
		{
			unlink(oldFile);
		}
		
		//ɾ����ǰ��һ��������״̬���� �����Ŀ¼�´����ļ��Ļ�
		memset(oldFile,0x00,100);
		if(1 == checkOldFile("/home/record/data",oldFile))
		{
			unlink(oldFile);
		}
		
	}
	
	return 0;
		
}

//����0��ʾDHCPģʽ  ����1��ʾ��̬IPģʽ
int get_DHCP_Status(void)
{
	int fd;
	fd = open("/config/staticIP.con", O_RDONLY, 0);
	if (fd >= 0)
	{
		close(fd);
		return 1;
	}else
	{
		return 0;
	}

}

void delete_newline(char *s)
{
	if(10 == s[strlen(s)-1])
		s[strlen(s)-1] = '\0';
}

int file_get_array(MyArray *array, int num, const char *filename)
{
	FILE *fp;
	int count = 0;
	char buffer[128] = {'\0'};
	memset(array, 0 ,sizeof(MyArray)*num);
	fp = fopen(filename, "r");
	if(fp == NULL){
		printmsg(ECU_DBG_CONTROL_CLIENT,(char *)filename);
		return -1;
	}
	while(!feof(fp))
	{
		if(count >= num)
		{
			fclose(fp);
			return 0;
		}
		memset(buffer, 0 ,sizeof(buffer));
		fgets(buffer, 128, fp);
		if(!strlen(buffer))continue;

		strncpy(array[count].name, buffer, strcspn(buffer, "="));
		strncpy(array[count].value, &buffer[strlen(array[count].name)+1], 64);
		delete_newline(array[count].value);
		count++;
	}
	fclose(fp);
	return 0;
}


float get_lifetime_power(void)
{
	int fd;
	float lifetime_power = 0.0;
	char buff[20] = {'\0'};
	fd = open("/HOME/DATA/LTPOWER", O_RDONLY, 0);
	if (fd >= 0)
	{
		memset(buff, '\0', sizeof(buff));
		read(fd, buff, 20);
		close(fd);
		
		if('\n' == buff[strlen(buff)-1])
			buff[strlen(buff)-1] = '\0';
		lifetime_power = (float)atof(buff);
	}		
	return lifetime_power;
}

void update_life_energy(float lifetime_power)
{
	int fd;
	char buff[20] = {'\0'};
	fd = open("/HOME/DATA/LTPOWER", O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) {
		sprintf(buff, "%f", lifetime_power);
		write(fd, buff, 20);
		close(fd);
	}
}

void save_system_power(int system_power, char *date_time)
{
	char dir[50] = "/home/record/power/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	int fd;
	if(system_power == 0) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],6);
	file[6] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	date_time_tmp[12] = '\0';
	print2msg(ECU_DBG_FILE,"save_system_power DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%d\n",date_time_tmp,system_power);
			//print2msg(ECU_DBG_MAIN,"save_system_power",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

//��������ǰ������
int calculate_earliest_2_day_ago(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	char day_s[3] = {'\0'};
	int year = 0,month = 0,day = 0;	//yearΪ��� monthΪ�·� dayΪ����  flag Ϊ�����жϱ�־ count��ʾ�ϸ��º���Ҫ�������� number_of_days:�ϸ��µ�������
	int flag = 0;
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);

	memcpy(day_s,&date[6],2);
	day_s[2] = '\0';
	day = atoi(day_s);

	if(day >= 3)
	{	//����ǰ�ڵ�ǰ��
		day -= 2;
		*earliest_data = (year * 10000 + month*100 + day);
		return 0;
	}else
	{	//����ǰ���ϸ���
		month -= 1;
		if(month == 0)
		{
			month = 12;
			year -= 1;
		}

		//������
		flag = leap(year);
		day = day_tab[flag][month-1]-(2 - day);
		*earliest_data = (year * 10000 + month*100 + day);
	}
	
	return -1;
}



//����������ǰ���·�
int calculate_earliest_2_month_ago(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	int year = 0,month = 0;	//yearΪ��� monthΪ�·� dayΪ����  flag Ϊ�����жϱ�־ count��ʾ�ϸ��º���Ҫ�������� number_of_days:�ϸ��µ�������
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);
	
	if(month >= 3)
	{
		month -= 2;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}else if(month == 2)
	{
		month = 12;
		year = year - 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}else if(month == 1)
	{
		month = 11;
		year = year - 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}
	
	return -1;
}

//ɾ������֮ǰ������
void delete_collect_info_2_day_ago(char *date_time)
{
	DIR *dirp;
	char dir[30] = "/home/record/rsdinfo";
	struct dirent *d;
	char path[100];
	int earliest_data,file_data;
	char fileTime[20] = {'\0'};

	/* ��dirĿ¼*/
	dirp = opendir("/home/record/rsdinfo");
	if(dirp == RT_NULL)
	{
		printmsg(ECU_DBG_CLIENT,"delete_collect_info_2_day_ago open directory error");
	}
	else
	{
		calculate_earliest_2_day_ago(date_time,&earliest_data);
		/* ��ȡdirĿ¼*/
		while ((d = readdir(dirp)) != RT_NULL)
		{
			memcpy(fileTime,d->d_name,8);
			fileTime[8] = '\0';
			file_data = atoi(fileTime);
			if(file_data <= earliest_data)
			{
				sprintf(path,"%s/%s",dir,d->d_name);
				unlink(path);
			}

			
		}
		/* �ر�Ŀ¼ */
		closedir(dirp);
	}


}


//ɾ��������֮ǰ������
void delete_system_power_2_month_ago(char *date_time)
{
	DIR *dirp;
	char dir[30] = "/home/record/power";
	struct dirent *d;
	char path[100];
	int earliest_data,file_data;
	char fileTime[20] = {'\0'};

	/* ��dirĿ¼*/
	dirp = opendir("/home/record/power");
	if(dirp == RT_NULL)
	{
		printmsg(ECU_DBG_CLIENT,"delete_system_power_2_month_ago open directory error");
	}
	else
	{
		calculate_earliest_2_month_ago(date_time,&earliest_data);
		//printf("calculate_earliest_2_month_ago:::::%d\n",earliest_data);
		/* ��ȡdirĿ¼*/
		while ((d = readdir(dirp)) != RT_NULL)
		{
		
			
			memcpy(fileTime,d->d_name,6);
			fileTime[6] = '\0';
			file_data = atoi(fileTime);
			if(file_data <= earliest_data)
			{
				sprintf(path,"%s/%s",dir,d->d_name);
				unlink(path);
			}

			
		}
		/* �ر�Ŀ¼ */
		closedir(dirp);
	}


}

//��ȡĳ�յĹ������߲���   ����   ����
int read_system_power(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/power";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int power = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(power_buff,date_time,8);
	memcpy(date_tmp,date_time,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);
	*length = 8;
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	//��8���ֽ�Ϊ��  ��ʱ����ͬ
			if((buff[12] == ',') && (!memcmp(buff,date_time,8)))
			{
				power = (int)atoi(&buff[13]);
				power_buff[(*length)++] = (buff[8]-'0') * 16 + (buff[9]-'0');
				power_buff[(*length)++] = (buff[10]-'0') * 16 + (buff[11]-'0');
				power_buff[(*length)++] = power/256;
				power_buff[(*length)++] = power%256;
			}
				
		}
		fclose(fp);
		return 0;
	}

	return -1;
}


int search_daily_energy(char *date,float *daily_energy)	
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);
	
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	//��8���ֽ�Ϊ��  ��ʱ����ͬ
			if((buff[8] == ',') && (!memcmp(buff,date,8)))
			{
				*daily_energy = (float)atof(&buff[9]);
				fclose(fp);
				rt_mutex_release(record_data_lock);
				return 0;
			}
				
		}
	
		fclose(fp);
	}

	rt_mutex_release(record_data_lock);

	return -1;
}


void update_daily_energy(float current_energy, char *date_time)
{
	char dir[50] = "/home/record/energy/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	float energy_tmp = current_energy;
	int fd;
	//��ǰһ�ַ�����Ϊ0 �����·�����
	if(current_energy <= EPSILON && current_energy >= -EPSILON) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],6);
	file[6] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	date_time_tmp[8] = '\0';	//�洢ʱ��Ϊ������ ����:20170718

	//����Ƿ��Ѿ����ڸ�ʱ��������
	if (0 == search_daily_energy(date_time_tmp,&energy_tmp))
	{
		energy_tmp = current_energy + energy_tmp;
		delete_line(dir,"/home/record/energy/1.tmp",date_time,8);
	}
	
	print2msg(ECU_DBG_FILE,"update_daily_energy DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{	
			ecu.today_energy = energy_tmp;
			sprintf(sendbuff,"%s,%f\n",date_time_tmp,energy_tmp);
			//print2msg(ECU_DBG_MAIN,"update_daily_energy",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

//����һ����������һ���ʱ��
int calculate_earliest_month(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	int year = 0,month = 0,day = 0;	//yearΪ��� monthΪ�·� dayΪ����  flag Ϊ�����жϱ�־ count��ʾ�ϸ��º���Ҫ�������� number_of_days:�ϸ��µ�������
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);

	day = 1;
	//����·�Ϊ1�� 
	if(month == 1)
	{
		year -= 1;
		month = 12;
		*earliest_data = (year * 10000 + month * 100 + day);
	}else
	{
		month -= 1;
		*earliest_data = (year * 10000 + month * 100 + day);
	}

	return 1;
	
}

//��ȡ���һ���µķ�����    ��ÿ�ռ���
int read_monthly_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int earliest_date = 0,compare_time = 0,flag = 0;
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	//����ǰ�����������һ��   ���flagΪ0  ��ʾ����һ���ڵ���  ���Ϊ1��ʾ���ϸ���
	flag = calculate_earliest_month(date_time,&earliest_date);
	
	if(flag == 1)
	{
		sprintf(date_tmp,"%d",earliest_date);
		date_tmp[6] = '\0';
		//����ļ�Ŀ¼
		sprintf(path,"%s/%s.dat",dir,date_tmp);
		//���ļ�
		//printf("path:%s\n",path);
		fp = fopen(path, "r");
		if(fp)
		{
			while(NULL != fgets(buff,100,fp))  //��ȡһ������
			{	
				//��ʱ��ת��Ϊint��   Ȼ����бȽ�
				memcpy(date_tmp,buff,8);
				date_tmp[8] = '\0';
				compare_time = atoi(date_tmp);
				//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
				if(compare_time >= earliest_date)
				{
					memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
					energy = (int)(atof(energy_tmp)*100);
					//printf("buff:%s\n energy:%d\n",buff,energy);
					power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
					power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
					power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
					power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
					power_buff[(*length)++] = energy/256;
					power_buff[(*length)++] = energy%256;
				}
					
			}
			fclose(fp);
		}
	}
	
	
	memcpy(date_tmp,date_time,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);

	//printf("path:%s\n",path);
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	
			//��ʱ��ת��Ϊint��   Ȼ����бȽ�
			memcpy(date_tmp,buff,8);
			date_tmp[8] = '\0';
			compare_time = atoi(date_tmp);
			//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
			if(compare_time >= earliest_date)
			{
				memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
				energy = (int)(atof(energy_tmp)*100);
				//printf("buff:%s\n energy:%d\n",buff,energy);
				power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
				power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
				power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
				power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
				power_buff[(*length)++] = energy/256;
				power_buff[(*length)++] = energy%256;
			}
				
		}
		fclose(fp);
	}
	return 0;
}

//����һ��������һ���µ�ʱ��
int calculate_earliest_year(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	int year = 0,month = 0;	//yearΪ��� monthΪ�·� dayΪ����  flag Ϊ�����жϱ�־ count��ʾ�ϸ��º���Ҫ�������� number_of_days:�ϸ��µ�������
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);
	month = 1;

	year -= 1;
	*earliest_data = (year * 100 + month);
	return 1;	
}

//��ȡ���һ��ķ�����    ��ÿ�¼���
int read_yearly_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int earliest_date = 0,compare_time = 0;
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	//����ǰ�����������һ��   ���flagΪ0  ��ʾ����һ���ڵ���  ���Ϊ1��ʾ���ϸ���
	calculate_earliest_year(date_time,&earliest_date);
	
	sprintf(path,"%s/year.dat",dir);

	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	
			//��ʱ��ת��Ϊint��   Ȼ����бȽ�
			memcpy(date_tmp,buff,6);
			date_tmp[6] = '\0';
			compare_time = atoi(date_tmp);
			//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
			if(compare_time >= earliest_date)
			{
				memcpy(energy_tmp,&buff[7],(strlen(buff)-7));
				energy = (int)(atof(energy_tmp)*100);
				//printf("buff:%s\n energy:%d\n",buff,energy);
				power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
				power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
				power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
				power_buff[(*length)++] = 0x01;
				power_buff[(*length)++] = energy/256;
				power_buff[(*length)++] = energy%256;
			}
				
		}
		fclose(fp);
	}
	return 0;
}


//��ȡ���һ��ķ�����    ��ÿ�¼���
int read_history_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	
	sprintf(path,"%s/history.dat",dir);

	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	
			//��ʱ��ת��Ϊint��   Ȼ����бȽ�
			memcpy(date_tmp,buff,4);
			date_tmp[4] = '\0';

			memcpy(energy_tmp,&buff[5],(strlen(buff)-5));
			energy = (int)(atof(energy_tmp)*100);
			//printf("buff:%s\n energy:%d\n",buff,energy);
			power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
			power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
			power_buff[(*length)++] = 0x01;
			power_buff[(*length)++] = 0x01;
			power_buff[(*length)++] = energy/256;
			power_buff[(*length)++] = energy%256;

				
		}
		fclose(fp);
	}
	return 0;
}



//��ȡĳ��ĳ��RSD����ز�������
int read_RSD_info(char *date_time,char * UID,char *rsd_buff,int *length)
{
	char dir[30] = "/home/record/rsdinfo";
	char UID_str[13] ={'\0'};
	char path[100];
	char buff[100]={'\0'};
	FILE *fp;
	char list[8][32];
	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	sprintf(UID_str,"%02x%02x%02x%02x%02x%02x",UID[0],UID[1],UID[2],UID[3],UID[4],UID[5]);
	UID_str[12] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_time);
	///home/record/rsdinfo/20170923.dat
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	
			//�ж�ID�Ƿ���ͬ
			if(!memcmp(buff,UID_str,12))
			{
				splitString(buff,list);
				rsd_buff[(*length)++] = (list[1][8]-'0')*16 + (list[1][9]-'0');
				rsd_buff[(*length)++] = (list[1][10]-'0')*16 + (list[1][11]-'0');
				rsd_buff[(*length)++] = atoi(list[2])/256;
				rsd_buff[(*length)++] = atoi(list[2])%256;
				rsd_buff[(*length)++] = atoi(list[3]);
				rsd_buff[(*length)++] = atoi(list[4])/256;
				rsd_buff[(*length)++] = atoi(list[4])/256;
				rsd_buff[(*length)++] = atoi(list[5])/256;
				rsd_buff[(*length)++] = atoi(list[5])%256;
				rsd_buff[(*length)++] = atoi(list[6]);
				rsd_buff[(*length)++] = atoi(list[7])/256;
				rsd_buff[(*length)++] = atoi(list[7])/256;
			}				
		}
		fclose(fp);
	}
	return 0;
}


int search_monthly_energy(char *date,float *daily_energy)	
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date,6);
	date_tmp[4] = '\0';
	sprintf(path,"%s/year.dat",dir);
	
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	//��8���ֽ�Ϊ��  ��ʱ����ͬ
			if((buff[6] == ',') && (!memcmp(buff,date,6)))
			{
				*daily_energy = (float)atof(&buff[7]);
				fclose(fp);
				rt_mutex_release(record_data_lock);
				return 0;
			}
				
		}
	
		fclose(fp);
	}

	rt_mutex_release(record_data_lock);

	return -1;
}

void update_monthly_energy(float current_energy, char *date_time)
{
	char dir[50] = "/home/record/energy/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	float energy_tmp = current_energy;
	int fd;
	//��ǰһ�ַ�����Ϊ0 �����·�����
	if(current_energy <= EPSILON && current_energy >= -EPSILON) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],4);
	file[4] = '\0';
	sprintf(dir,"%syear.dat",dir);
	date_time_tmp[6] = '\0';	//�洢ʱ��Ϊ������ ����:20170718

	//����Ƿ��Ѿ����ڸ�ʱ��������
	if (0 == search_monthly_energy(date_time_tmp,&energy_tmp))
	{
		energy_tmp = current_energy + energy_tmp;
		delete_line(dir,"/home/record/energy/2.tmp",date_time,6);
	}
	
	print2msg(ECU_DBG_FILE,"update_monthly_energy DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%f\n",date_time_tmp,energy_tmp);
			//print2msg(ECU_DBG_MAIN,"update_daily_energy",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

int search_yearly_energy(char *date,float *daily_energy)	
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date,4);
	date_tmp[4] = '\0';
	sprintf(path,"%s/history.dat",dir);
	
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //��ȡһ������
		{	//��8���ֽ�Ϊ��  ��ʱ����ͬ
			if((buff[4] == ',') && (!memcmp(buff,date,4)))
			{
				*daily_energy = (float)atof(&buff[5]);
				fclose(fp);
				rt_mutex_release(record_data_lock);
				return 0;
			}
				
		}
	
		fclose(fp);
	}

	rt_mutex_release(record_data_lock);

	return -1;
}


void update_yearly_energy(float current_energy, char *date_time)
{
	char dir[50] = "/home/record/energy/";
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	float energy_tmp = current_energy;
	int fd;
	//��ǰһ�ַ�����Ϊ0 �����·�����
	if(current_energy <= EPSILON && current_energy >= -EPSILON) return;
	
	memcpy(date_time_tmp,date_time,14);
	sprintf(dir,"%shistory.dat",dir);
	date_time_tmp[4] = '\0';	//�洢ʱ��Ϊ�� ����:2017

	//����Ƿ��Ѿ����ڸ�ʱ��������
	if (0 == search_yearly_energy(date_time_tmp,&energy_tmp))
	{
		energy_tmp = current_energy + energy_tmp;
		delete_line(dir,"/home/record/energy/3.tmp",date_time,4);
	}
	
	print2msg(ECU_DBG_FILE,"update_yearly_energy DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%f\n",date_time_tmp,energy_tmp);
			//print2msg(ECU_DBG_MAIN,"update_daily_energy",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}


void save_dbg(char sendbuff[])
{
	char file[20] = "/dbg/dbg.dat";
	rt_err_t result;
	int fd;
	
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(file, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s\n",sendbuff);
			print2msg(ECU_DBG_OTHER,"save_dbg",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}


void save_record(char sendbuff[], char *date_time)
{
	char dir[50] = "/home/record/data/";
	char file[9];
	rt_err_t result;
	int fd;
	
	memcpy(file,&date_time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_FILE,"save_record DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%s,1\n",sendbuff,date_time);
			//print2msg(ECU_DBG_MAIN,"save_record",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}


//�������һ��ͨѶ�������
void save_last_collect_info(void)
{
	int fd;
	char str[300] = {'\0'};
	inverter_info *curinverter = inverterInfo;
	int i = 0;
	fd = fileopen("/home/data/collect.con",O_WRONLY | O_APPEND | O_CREAT|O_TRUNC,0);
	for(i = 0;i< ecu.validNum; i++)
	{
		if(curinverter->status.comm_status == 1)
		{
			sprintf(str,"%02x%02x%02x%02x%02x%02x,%s,%d,%d\n",curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5],curinverter->LastCollectTime,curinverter->Last_PV1_Energy,curinverter->Last_PV2_Energy);
			fileWrite(fd,str,strlen(str));
		}
		curinverter++;
	}
	fileclose(fd);

}

//������ʷ�����������
void save_collect_info(char *curTime)
{
	int fd;
	char str[300] = {'\0'};
	inverter_info *curinverter = inverterInfo;
	int i = 0;
	char path[60] = {'\0'};
	char pathTime[9] = {'\0'};
	
	memcpy(pathTime,curTime,8);
	pathTime[8] = '\0';
	sprintf(path,"/home/record/rsdinfo/%s.dat",pathTime);
	print2msg(ECU_DBG_COLLECT,"path:",path);
	
	fd = fileopen(path,O_WRONLY | O_APPEND | O_CREAT,0);
	for(i = 0;i< ecu.validNum; i++)
	{
		if(curinverter->status.comm_status == 1)
		{
			memset(str,'\0',300);
			sprintf(str,"%02x%02x%02x%02x%02x%02x,%s,%d,%d,%d,%d,%d,%d\n",
				curinverter->uid[0],curinverter->uid[1],curinverter->uid[2],curinverter->uid[3],curinverter->uid[4],curinverter->uid[5],curTime,
				curinverter->PV1,curinverter->PI,(int)curinverter->AveragePower1,curinverter->PV2,curinverter->PI,(int)curinverter->AveragePower2);
			fileWrite(fd,str,strlen(str));
		}
		
	
		curinverter++;
	}
	fileclose(fd);

}

int detection_resendflag2(void)		//���ڷ���1�������ڷ���0
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(buff,'\0',CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"detection_resendflag2 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//print2msg(ECU_DBG_CLIENT,"detection_resendflag2",path);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))
					{
						//����Ƿ���ϸ�ʽ
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] == '2')			//������һ���ֽڵ�resendflag�Ƿ�Ϊ2   ���������2  �ر��ļ�����return 1
							{
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}		
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
}


int change_resendflag(char *time,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(buff,'\0',CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"change_resendflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							memset(filetime,0,15);
							memcpy(filetime,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
							filetime[14] = '\0';
							if(!memcmp(time,filetime,14))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
							{
								fseek(fp,-2L,SEEK_CUR);
								fputc(flag,fp);
								//print2msg(ECU_DBG_CLIENT,"change_resendflag",filetime);
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
	
}	


//��ѯһ��resendflagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
time����ʾ��ȡ����ʱ��
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_readflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int nextfileflag = 0;	//0��ʾ��ǰ�ļ��ҵ������ݣ�1��ʾ��Ҫ�Ӻ�����ļ���������
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(buff,'\0',CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	*flag = 0;
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"search_readflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					if(0 == nextfileflag)
					{
						while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))  //��ȡһ������
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
								{
									memcpy(time,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
									memcpy(data,buff,(strlen(buff)-18));
									data[strlen(buff)-18] = '\n';
									//print2msg(ECU_DBG_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CLIENT,"search_readflag data",data);
									rt_hw_s_delay(1);
									while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
									{
										if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
										{
											if(buff[strlen(buff)-2] == sendflag)
											{
												*flag = 1;
												fclose(fp);
												closedir(dirp);
												free(buff);
												buff = NULL;
												rt_mutex_release(record_data_lock);
												return 1;
											}
										}			
									}

									nextfileflag = 1;
									break;
								}
							}
								
						}
					}else
					{
						while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))  //��ȡһ������
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									free(buff);
									buff = NULL;
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);

	return nextfileflag;
}


//�������resend��־ȫ��Ϊ0��Ŀ¼
void delete_file_resendflag0(void)		
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(buff,'\0',CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"delete_file_resendflag0 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				flag = 0;
				//print2msg(ECU_DBG_CLIENT,"delete_file_resendflag0 ",path);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
							{
								flag = 1;
								break;
							}
						}
						
					}
					fclose(fp);
					if(flag == 0)
					{
						print2msg(ECU_DBG_CLIENT,"unlink:",path);
						//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
						unlink(path);
					}	
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return;

}

int clear_send_flag(char *readbuff)
{
	int i, j, count;		//EMA���ض��ٸ�ʱ��(�м���ʱ��,��˵��EMA�����˶�������¼)
	char recv_date_time[16];
	
	if(strlen(readbuff) >= 3)
	{
		count = (readbuff[1] - 0x30) * 10 + (readbuff[2] - 0x30);
		if(count == (strlen(readbuff) - 3) / 14)
		{
			for(i=0; i<count; i++)
			{
				memset(recv_date_time, '\0', sizeof(recv_date_time));
				strncpy(recv_date_time, &readbuff[3+i*14], 14);
				
				for(j=0; j<3; j++)
				{
					if(1 == change_resendflag(recv_date_time,'0'))
					{
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "1");
						break;
					}
					else
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "0");
				}
			}
		}
	}

	return 0;
}

int update_send_flag(char *send_date_time)
{
	int i;
	for(i=0; i<3; i++)
	{
		if(1 == change_resendflag(send_date_time,'2'))
		{
			print2msg(ECU_DBG_CLIENT,"Update send flag into database", "1");
			break;
		}
		rt_hw_s_delay(5);
	}

	return 0;
}


void save_control_record(char sendbuff[], char *date_time)
{
	char dir[50] = "/home/record/ctldata/";
	char file[9];
	rt_err_t result;
	int fd;
	
	memcpy(file,&date_time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_FILE,"save_record DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%s,1\n",sendbuff,date_time);
			//print2msg(ECU_DBG_MAIN,"save_record",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}


int detection_control_resendflag2(void)		//���ڷ���1�������ڷ���0
{
	DIR *dirp;
	char dir[30] = "/home/record/ctldata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/ctldata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"detection_resendflag2 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//print2msg(ECU_DBG_CLIENT,"detection_resendflag2",path);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						//����Ƿ���ϸ�ʽ
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] == '2')			//������һ���ֽڵ�resendflag�Ƿ�Ϊ2   ���������2  �ر��ļ�����return 1
							{
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}		
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
}


int change_control_resendflag(char *time,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
{
	DIR *dirp;
	char dir[30] = "/home/record/ctldata/";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/ctldata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"change_resendflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							memset(filetime,0,15);
							memcpy(filetime,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
							filetime[14] = '\0';
							if(!memcmp(time,filetime,14))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
							{
								fseek(fp,-2L,SEEK_CUR);
								fputc(flag,fp);
								//print2msg(ECU_DBG_CLIENT,"change_resendflag",filetime);
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
	
}	


//��ѯһ��resendflagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
time����ʾ��ȡ����ʱ��
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_control_readflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/ctldata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int nextfileflag = 0;	//0��ʾ��ǰ�ļ��ҵ������ݣ�1��ʾ��Ҫ�Ӻ�����ļ���������
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	*flag = 0;
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/ctldata/");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"search_readflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					if(0 == nextfileflag)
					{
						while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))  //��ȡһ������
						{
							
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
								{
									memcpy(time,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
									memcpy(data,buff,(strlen(buff)-18));
									data[strlen(buff)-18] = '\n';
									//print2msg(ECU_DBG_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CLIENT,"search_readflag data",data);
									rt_hw_s_delay(1);
									while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
									{
										if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
										{
											if(buff[strlen(buff)-2] == sendflag)
											{
												*flag = 1;
												fclose(fp);
												closedir(dirp);
												free(buff);
												buff = NULL;
												rt_mutex_release(record_data_lock);
												return 1;
											}
										}			
									}

									nextfileflag = 1;
									break;
								}
							}
								
						}
					}else
					{
						while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))  //��ȡһ������
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									free(buff);
									buff = NULL;
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);

	return nextfileflag;
}


//�������resend��־ȫ��Ϊ0��Ŀ¼
void delete_control_file_resendflag0(void)		
{
	DIR *dirp;
	char dir[30] = "/home/record/ctldata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/ctldata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"delete_file_resendflag0 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				flag = 0;
				//print2msg(ECU_DBG_CLIENT,"delete_file_resendflag0 ",path);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
							{
								flag = 1;
								break;
							}
						}
						
					}
					fclose(fp);
					if(flag == 0)
					{
						print2msg(ECU_DBG_CLIENT,"unlink:",path);
						//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
						unlink(path);
					}	
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return;

}

int clear_control_send_flag(char *readbuff)
{
	int i, j, count;		//EMA���ض��ٸ�ʱ��(�м���ʱ��,��˵��EMA�����˶�������¼)
	char recv_date_time[16];
	
	if(strlen(readbuff) >= 3)
	{
		count = (readbuff[1] - 0x30) * 10 + (readbuff[2] - 0x30);
		if(count == (strlen(readbuff) - 3) / 14)
		{
			for(i=0; i<count; i++)
			{
				memset(recv_date_time, '\0', sizeof(recv_date_time));
				strncpy(recv_date_time, &readbuff[3+i*14], 14);
				
				for(j=0; j<3; j++)
				{
					if(1 == change_control_resendflag(recv_date_time,'0'))
					{
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "1");
						break;
					}
					else
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "0");
				}
			}
		}
	}

	return 0;
}

int update_control_send_flag(char *send_date_time)
{
	int i;
	for(i=0; i<3; i++)
	{
		if(1 == change_control_resendflag(send_date_time,'2'))
		{
			print2msg(ECU_DBG_CLIENT,"Update send flag into database", "1");
			break;
		}
		rt_hw_s_delay(5);
	}

	return 0;
}

//����������Ϣ	mos_status״̬ͨ�������ѹ�жϡ������ѹ����0 ��ʾMOS�ܿ����������ѹС��0 ��ʾMOS�ܹرա�
void create_alarm_record(unsigned short last_PV_output,unsigned char last_function_status,unsigned char last_pv1_low_voltage_pritection,unsigned char last_pv2_low_voltage_pritection,inverter_info *curinverter)
{
#if 1
	int create_flag = 0;
	char *alarm_data = 0;
	char curTime[15] = {'\0'};
	int length = 0;
	unsigned char last_mos_status = 0,mos_status = 0;
	if(last_PV_output > 0) 
		last_mos_status = 1;
	else 
		last_mos_status = 0;

	if(curinverter->PV_Output > 0)
		mos_status = 1;
	else
		mos_status = 0;
	
	alarm_data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	
	if((last_mos_status != mos_status) || (last_function_status != curinverter->status.function_status) || (last_pv1_low_voltage_pritection != curinverter->status.pv1_low_voltage_pritection) || ((last_pv2_low_voltage_pritection != curinverter->status.pv2_low_voltage_pritection)))
	{
		//���������һ�ֲ�ͬ��״̬����Ҫ����״̬�澯��Ϣ
		create_flag = 1;
	}

	if(create_flag == 1)
	{
		apstime(curTime);
		curTime[14] = '\0';
		//ͷ��Ϣ
		memcpy(alarm_data,"APS13AAAAAA156AAA1",18);
		//ECUͷ��Ϣ
		memcpy(&alarm_data[18],ecu.ECUID12,12);
		memcpy(&alarm_data[30],"0001",4);
		memcpy(&alarm_data[34],curTime,14);
		memcpy(&alarm_data[48],"END",3);
		
		length = 51;
		
		alarm_data[length++] = (curinverter->uid[0]/16) + '0';
		alarm_data[length++] = (curinverter->uid[0]%16) + '0';
		alarm_data[length++] = (curinverter->uid[1]/16) + '0';
		alarm_data[length++] = (curinverter->uid[1]%16) + '0';
		alarm_data[length++] = (curinverter->uid[2]/16) + '0';
		alarm_data[length++] = (curinverter->uid[2]%16) + '0';
		alarm_data[length++] = (curinverter->uid[3]/16) + '0';
		alarm_data[length++] = (curinverter->uid[3]%16) + '0';
		alarm_data[length++] = (curinverter->uid[4]/16) + '0';
		alarm_data[length++] = (curinverter->uid[4]%16) + '0';
		alarm_data[length++] = (curinverter->uid[5]/16) + '0';
		alarm_data[length++] = (curinverter->uid[5]%16) + '0';
		alarm_data[length++] = mos_status + '0';
		alarm_data[length++] = curinverter->status.function_status + '0';
		alarm_data[length++] = curinverter->status.pv1_low_voltage_pritection+ '0';
		alarm_data[length++] = curinverter->status.pv2_low_voltage_pritection+ '0';
		
		memcpy(&alarm_data[length],"000000",6);
		length += 6;

		alarm_data[length++] = 'E';
		alarm_data[length++] = 'N';
		alarm_data[length++] = 'D';

		alarm_data[5] = (length/10000)%10 + 0x30;
		alarm_data[6] = (length/1000)%10+ 0x30;
		alarm_data[7] = (length/100)%10+ 0x30;
		alarm_data[8] = (length/10)%10+ 0x30;
		alarm_data[9] = (length/1)%10+ 0x30;
		
		alarm_data[length++] = '\0';
		
		save_alarm_record(alarm_data,curTime);
		print2msg(ECU_DBG_COMM,"alarm Data:",alarm_data);
	}

	free(alarm_data);
	alarm_data = NULL;
	#endif
}

void save_alarm_record(char sendbuff[], char *date_time)
{
	char dir[50] = "/home/record/almdata/";
	char file[9];
	rt_err_t result;
	int fd;
	
	memcpy(file,&date_time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_FILE,"save_record DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%s,1\n",sendbuff,date_time);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}


int detection_alarm_resendflag2(void)		//���ڷ���1�������ڷ���0
{
	DIR *dirp;
	char dir[30] = "/home/record/almdata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/almdata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"detection_resendflag2 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//print2msg(ECU_DBG_CLIENT,"detection_resendflag2",path);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						//����Ƿ���ϸ�ʽ
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] == '2')			//������һ���ֽڵ�resendflag�Ƿ�Ϊ2   ���������2  �ر��ļ�����return 1
							{
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}		
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
}


int change_alarm_resendflag(char *time,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
{
	DIR *dirp;
	char dir[30] = "/home/record/almdata/";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/almdata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"change_resendflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							memset(filetime,0,15);
							memcpy(filetime,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
							filetime[14] = '\0';
							if(!memcmp(time,filetime,14))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
							{
								fseek(fp,-2L,SEEK_CUR);
								fputc(flag,fp);
								//print2msg(ECU_DBG_CLIENT,"change_resendflag",filetime);
								fclose(fp);
								closedir(dirp);
								free(buff);
								buff = NULL;
								rt_mutex_release(record_data_lock);
								return 1;
							}
						}
					}
					fclose(fp);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return 0;
	
}	


//��ѯһ��resendflagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
time����ʾ��ȡ����ʱ��
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_alarm_readflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/almdata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int nextfileflag = 0;	//0��ʾ��ǰ�ļ��ҵ������ݣ�1��ʾ��Ҫ�Ӻ�����ļ���������
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	*flag = 0;
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/almdata/");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"search_readflag open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					if(0 == nextfileflag)
					{
						while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))  //��ȡһ������
						{
							
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
								{
									memcpy(time,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
									memcpy(data,buff,(strlen(buff)-18));
									data[strlen(buff)-18] = '\n';
									//print2msg(ECU_DBG_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CLIENT,"search_readflag data",data);
									rt_hw_s_delay(1);
									while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
									{
										if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
										{
											if(buff[strlen(buff)-2] == sendflag)
											{
												*flag = 1;
												fclose(fp);
												closedir(dirp);
												free(buff);
												buff = NULL;
												rt_mutex_release(record_data_lock);
												return 1;
											}
										}			
									}

									nextfileflag = 1;
									break;
								}
							}
								
						}
					}else
					{
						while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))  //��ȡһ������
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									free(buff);
									buff = NULL;
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					
					fclose(fp);
				}
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);

	return nextfileflag;
}


//�������resend��־ȫ��Ϊ0��Ŀ¼
void delete_alarm_file_resendflag0(void)		
{
	DIR *dirp;
	char dir[30] = "/home/record/almdata/";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(buff,'\0',CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	if(result == RT_EOK)
	{
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/almdata/");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"delete_file_resendflag0 open directory error");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
				sprintf(path,"%s/%s",dir,d->d_name);
				flag = 0;
				//print2msg(ECU_DBG_CLIENT,"delete_file_resendflag0 ",path);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
							{
								flag = 1;
								break;
							}
						}
						
					}
					fclose(fp);
					if(flag == 0)
					{
						print2msg(ECU_DBG_CLIENT,"unlink:",path);
						//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
						unlink(path);
					}	
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return;

}

int clear_alarm_send_flag(char *readbuff)
{
	int i, j, count;		//EMA���ض��ٸ�ʱ��(�м���ʱ��,��˵��EMA�����˶�������¼)
	char recv_date_time[16];
	
	if(strlen(readbuff) >= 3)
	{
		count = (readbuff[1] - 0x30) * 10 + (readbuff[2] - 0x30);
		if(count == (strlen(readbuff) - 3) / 14)
		{
			for(i=0; i<count; i++)
			{
				memset(recv_date_time, '\0', sizeof(recv_date_time));
				strncpy(recv_date_time, &readbuff[3+i*14], 14);
				
				for(j=0; j<3; j++)
				{
					if(1 == change_alarm_resendflag(recv_date_time,'0'))
					{
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "1");
						break;
					}
					else
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "0");
				}
			}
		}
	}

	return 0;
}

int update_alarm_send_flag(char *send_date_time)
{
	int i;
	for(i=0; i<3; i++)
	{
		if(1 == change_alarm_resendflag(send_date_time,'2'))
		{
			print2msg(ECU_DBG_CLIENT,"Update send flag into database", "1");
			break;
		}
		rt_hw_s_delay(5);
	}

	return 0;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(initsystem, eg:initsystem("80:97:1B:00:72:1C"));
#endif
