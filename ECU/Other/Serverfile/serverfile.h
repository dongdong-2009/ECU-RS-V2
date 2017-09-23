#ifndef __SERVERFILE_H__
#define __SERVERFILE_H__
#include "variation.h"
#include <rtthread.h>


typedef struct name_value
{
	char name[32];
	char value[64];
}MyArray;


int fileopen(const char *file, int flags, int mode);
int fileclose(int fd);
int fileWrite(int fd,char* buf,int len);
int fileRead(int fd,char* buf,int len);

void init_tmpdb(inverter_info *firstinverter);
void init_RecordMutex(void);
void echo(const char* filename,const char* string);
void get_mac(rt_uint8_t  dev_addr[6]);
int initPath(void);
int initsystem(char *mac);
int get_DHCP_Status(void);
int file_get_array(MyArray *array, int num, const char *filename);
void save_last_collect_info(void);
void save_collect_info(char *curTime);
int delete_line(char* filename,char* temfilename,char* compareData,int len);
int search_line(char* filename,char* compareData,int len);
int insert_line(char * filename,char *str);
int optimizeFileSystem(void);
void splitSpace(char *data,char *sourcePath,char *destPath);
void save_dbg(char sendbuff[]);
int read_RSD_info(char *date_time,char * UID,char *rsd_buff,int *length);
//保存Client数据,以及其他相关Client的操作
float get_lifetime_power(void);
void update_life_energy(float lifetime_power);
void save_system_power(int system_power, char *date_time);
int calculate_earliest_2_day_ago(char *date,int *earliest_data);
void delete_collect_info_2_day_ago(char *date_time);
int calculate_earliest_2_month_ago(char *date,int *earliest_data);
void delete_system_power_2_month_ago(char *date_time);
int read_system_power(char *date_time, char *power_buff,int *length);
int search_daily_energy(char *date,float *daily_energy)	;
void update_daily_energy(float current_energy, char *date_time);
int calculate_earliest_month(char *date,int *earliest_data);
int read_monthly_energy(char *date_time, char *power_buff,int *length);
int calculate_earliest_year(char *date,int *earliest_data);
int read_yearly_energy(char *date_time, char *power_buff,int *length);

int read_history_energy(char *date_time, char *power_buff,int *length);
int search_monthly_energy(char *date,float *daily_energy);
void update_monthly_energy(float current_energy, char *date_time);
int search_yearly_energy(char *date,float *daily_energy);
void update_yearly_energy(float current_energy, char *date_time);

void save_record(char sendbuff[], char *date_time);
int detection_resendflag2(void);
int change_resendflag(char *time,char flag);
int search_readflag(char *data,char * time, int *flag,char sendflag);
void delete_file_resendflag0(void);
int clear_send_flag(char *readbuff);
int update_send_flag(char *send_date_time);

//保存15分钟远程控制数据以及其他远程控制相关操作
void save_control_record(char sendbuff[], char *date_time);
int detection_control_resendflag2(void);
int change_control_resendflag(char *time,char flag);
int search_control_readflag(char *data,char * time, int *flag,char sendflag);
void delete_control_file_resendflag0(void);
int clear_control_send_flag(char *readbuff);
int update_control_send_flag(char *send_date_time);


//远程控制告警信息
//生成告警标志
void create_alarm_record(unsigned short last_PV_output,unsigned char last_function_status,unsigned char last_pv1_low_voltage_pritection,unsigned char last_pv2_low_voltage_pritection,inverter_info *curinverter);
void save_alarm_record(char sendbuff[], char *date_time);
int detection_alarm_resendflag2(void);
int change_alarm_resendflag(char *time,char flag);
int search_alarm_readflag(char *data,char * time, int *flag,char sendflag);
void delete_alarm_file_resendflag0(void);
int clear_alarm_send_flag(char *readbuff);
int update_alarm_send_flag(char *send_date_time);

#endif /*__SERVERFILE_H__*/
