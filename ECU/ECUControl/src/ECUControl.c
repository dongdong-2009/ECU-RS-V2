#include "ECUControl.h"
#include "usart5.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "stdio.h"
#include "debug.h"
#include "threadlist.h"
#include "serverfile.h"
#include "remote_control_protocol.h"
#include "string.h"
#include "stdlib.h"
#include "rtc.h"
#include "socket.h"
#include "inverter_update.h"
#include "set_rsd_function_switch.h"
#include "inverter_id.h"
#include "custom_command.h"
#include <dfs_posix.h> 
#include "datelist.h"
#include "ZigBeeTransmission.h"
#include "ZigBeeChannel.h"

extern rt_mutex_t record_data_lock;
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150, A151, A152, A153, A154, A155, A156, A157, A158, A159,
	A160, A161, A162, A163, A164, A165, A166, A167, A168, A169,
	A170, A171, A172, A173, A174, A175, A176, A177, A178, A179,
};
int (*pfun[200])(const char *recvbuffer, char *sendbuffer);

void add_functions()
{
	pfun[A102] = response_inverter_id; 			//上报逆变器ID  										OK
	pfun[A103] = set_inverter_id; 				//设置逆变器ID												OK
	pfun[A108] = custom_command;			//向ECU发送自定义命令
	pfun[A136] = set_inverter_update;			//设置逆变器的升级标志		
	pfun[A160] = set_rsd_function_switch;		//RSD功能开关									OK
	pfun[A171] = set_ZigBeeChannel;			//设置信道
	pfun[A172] = response_ZigBeeChannel_Result;	//上报信道设置结果
	pfun[A173] = transmission_ZigBeeInfo;		//ZigBee报文透传
}

/* 与EMA进行通讯 */
int communication_with_EMA(int next_cmd_id)
{
	int sockfd;
	int cmd_id;
	char timestamp[15] = "00000000000000";	//时间戳
	char *recv_buffer = NULL;
	char *send_buffer = NULL;

	recv_buffer = rt_malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	send_buffer = rt_malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	
	while(1)
	{
		printmsg(ECU_DBG_CONTROL_CLIENT,"Start Communication with EMA");
		sockfd = Control_client_socket_init();
		if(sockfd < 0) 
		{
		
#ifdef WIFI_USE	

			//有线连接失败，使用wifi传输 
			{
				int j =0,flag_failed = 0,ret = 0;
				if(next_cmd_id <= 0)
				{
					//ECU向EMA发送请求命令指令
					msg_REQ(send_buffer);
					ret = SendToSocketC(control_client_arg.ip,randport(control_client_arg),send_buffer, strlen(send_buffer));
					if(ret == -1)
					{
						rt_free(recv_buffer);
						rt_free(send_buffer);
						return -1;
					}
					memset(send_buffer, '\0', sizeof(send_buffer));
					for(j = 0;j<800;j++)
					{
						if(WIFI_Recv_SocketC_Event == 1)
						{
							flag_failed = 1;
							WIFI_Recv_SocketC_Event = 0;
							break;
						}
						rt_thread_delay(1);
					}
					if(flag_failed == 0)
					{
						rt_free(recv_buffer);
						rt_free(send_buffer);
						return -1;
					}

					//去掉usr WIFI报文的头部
					memcpy(recv_buffer,WIFI_RecvSocketCData,WIFI_Recv_SocketC_LEN);
					recv_buffer[WIFI_Recv_SocketC_LEN] = '\0';
					print2msg(ECU_DBG_CONTROL_CLIENT,"communication_with_EMA recv",recv_buffer);
					//校验命令
					if(msg_format_check(recv_buffer) < 0){
						continue;
					}
					//解析命令号
					cmd_id = msg_cmd_id(recv_buffer);
				}
				else{
					//生成下一条命令(用于设置命令结束后,上报设置后的ECU状态)
					cmd_id = next_cmd_id;
					next_cmd_id = 0;
					memset(recv_buffer, 0, sizeof(recv_buffer));
					snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
							ecu.ECUID12, cmd_id, timestamp);
				}

				//根据命令号调用函数
				if(pfun[cmd_id%100]){
					//若设置函数调用完毕后需要执行上报,则会返回上报函数的命令号,否则返回0
					next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
				}
				//EMA命令发送完毕
				else if(cmd_id == 100){
					break;
				}
				else{
					//若命令号不存在,则发送设置失败应答(每条设置协议的时间戳位置不统一,返回时间戳是个问题...)
					memset(send_buffer, 0, sizeof(send_buffer));
					snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END\n",
							ecu.ECUID12, cmd_id);
				}
				//将消息发送给EMA(自动计算长度,补上回车)
				printf("%s\n",send_buffer);
				SendToSocketC(control_client_arg.ip,randport(control_client_arg),send_buffer, strlen(send_buffer));
				printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
				//如果功能函数返回值小于0,则返回-1,程序会自动退出
				if(next_cmd_id < 0){
					rt_free(recv_buffer);
					rt_free(send_buffer);
					return -1;
				}				
			}
#endif

#ifndef WIFI_USE
		break;
#endif
	
		}
		else
		{
			if(next_cmd_id <= 0)
			{
				//ECU向EMA发送请求命令指令
				msg_REQ(send_buffer);
				send_socket(sockfd, send_buffer, strlen(send_buffer));
				memset(send_buffer, '\0', sizeof(send_buffer));

				//接收EMA发来的命令
				if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), 10) < 0){
					close_socket(sockfd);
					rt_free(recv_buffer);
					rt_free(send_buffer);
					return -1;
				}
				//校验命令
				if(msg_format_check(recv_buffer) < 0){
					close_socket(sockfd);
					continue;
				}
				//解析命令号
				cmd_id = msg_cmd_id(recv_buffer);
			}
			else{
				//生成下一条命令(用于设置命令结束后,上报设置后的ECU状态)
				cmd_id = next_cmd_id;
				next_cmd_id = 0;
				memset(recv_buffer, 0, sizeof(recv_buffer));
				snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
						ecu.ECUID12, cmd_id, timestamp);
			}

			if(pfun[cmd_id%100]){
				//若设置函数调用完毕后需要执行上报,则会返回上报函数的命令号,否则返回0
				next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
			}
			//EMA命令发送完毕
			else if(cmd_id == 100){
				close_socket(sockfd);
				break;
			}
			else{
				//若命令号不存在,则发送设置失败应答(每条设置协议的时间戳位置不统一,返回时间戳是个问题...)
				memset(send_buffer, 0, sizeof(send_buffer));
				snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END",
						ecu.ECUID12, cmd_id);
			}
			//将消息发送给EMA(自动计算长度,补上回车)
			send_socket(sockfd, send_buffer, strlen(send_buffer));
			printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
			close_socket(sockfd);

			//如果功能函数返回值小于0,则返回-1,程序会自动退出
			if(next_cmd_id < 0){
				rt_free(recv_buffer);
				rt_free(send_buffer);
				return -1;
			}
		}
	}
	
	printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
	rt_free(recv_buffer);
	rt_free(send_buffer);
	return 0;
}

//发送A157命令 
int prealarmprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_alarm_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS1300047A157AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes > 54)
	{
		clear_alarm_send_flag(&readbuff[49]);
		
	}else
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	free(readbuff);
	readbuff = NULL;
	return 0;
}

int send_alarm_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	char *readbuff = NULL;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,'\0',(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes < 54)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		if('1' == readbuff[49])
			update_alarm_send_flag(send_date_time);
		clear_alarm_send_flag(&readbuff[49]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_alarm_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_alarm_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		//printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_alarm_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}

//发送A159命令 
int precontrolprocess(void)			
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char nowtime[15] = {'\0'};
	char sendbuff[50] = {'\0'};

	if(0 == detection_control_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,0x00,(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	
	apstime(nowtime);
	memcpy(sendbuff,"APS1300047A159AAA1",18);
	memcpy(&sendbuff[18],ecu.ECUID12,12);
	memcpy(&sendbuff[30],nowtime,14);
	memcpy(&sendbuff[44],"END\n",4);

	print2msg(ECU_DBG_CONTROL_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes > 54)
	{
		clear_control_send_flag(&readbuff[49]);
		
	}else
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	free(readbuff);
	readbuff = NULL;
	return 0;
}

int send_control_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = (CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER);
	char *readbuff = NULL;
	readbuff = malloc((CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	memset(readbuff,'\0',(CONTROL_RECORD_HEAD+CONTROL_RECORD_ALARM_ECU_HEAD+CONTROL_RECORD_OTHER));
	
	sendbytes = serverCommunication_Control(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes < 54)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		if('1' == readbuff[49])
			update_control_send_flag(send_date_time);
		clear_control_send_flag(&readbuff[49]);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int resend_control_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER));
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_control_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		//printmsg(ECU_DBG_CONTROL_CLIENT,data);
		res = send_control_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;
}

//根据时间点修改标志
int change_pro_result_flag(char *item,char flag)  //改变成功返回1，未找到该时间点返回0
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char fileitem[4] = {'\0'};
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"change_pro_result_flag open directory error");
			mkdir("/home/data/proc_res",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			//打开文件一行行判断是否有flag=2的  如果存在直接关闭文件并返回1
			fp = fopen(path, "r+");
			if(fp)
			{
				while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
				{
					if(strlen(buff) > 18)
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') )
						{
							if(buff[strlen(buff)-2] == '1')
							{
								memset(fileitem,0,4);
								memcpy(fileitem,&buff[strlen(buff)-6],3);				//获取每条记录的时间
								fileitem[3] = '\0';
								if(!memcmp(item,fileitem,4))						//每条记录的时间和传入的时间对比，若相同则变更flag				
								{
									fseek(fp,-2L,SEEK_CUR);
									fputc(flag,fp);
									//print2msg(ECU_DBG_CONTROL_CLIENT,"filetime",filetime);
									fclose(fp);
									free(buff);
									buff = NULL;
									list_destroy(&Head);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					
				}
				fclose(fp);
			}
		}
	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);
	return 0;
	
}

void delete_pro_result_flag0()		//清空数据flag标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"delete_pro_result_flag0 open directory error");
			mkdir("/home/data/proc_res",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			flag = 0;
			//print2msg(ECU_DBG_CONTROL_CLIENT,"delete_file_resendflag0 ",path);
			//打开文件一行行判断是否有flag!=0的  如果存在直接关闭文件并返回,如果不存在，删除文件
			fp = fopen(path, "r");
			if(fp)
			{
				while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
				{
					if(strlen(buff) > 18)
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') )
						{
							if(buff[strlen(buff)-2] != '0')			//检测是否存在resendflag != 0的记录   若存在则直接退出函数
							{
								flag = 1;
								break;

							}
						}
					}
					memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					
				}
				fclose(fp);
				if(flag == 0)
				{
					print2msg(ECU_DBG_CONTROL_CLIENT,"unlink:",path);
					//遍历完文件都没发现flag != 0的记录直接删除文件
					unlink(path);
				}	
			}
		}
	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);
	return;

}

void delete_inv_pro_result_flag0()		//清空数据flag标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/data/iprocres";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/iprocres");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"delete_inv_pro_result_flag0 open directory error");
			mkdir("/home/data/iprocres",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			flag = 0;
			//print2msg(ECU_DBG_CONTROL_CLIENT,"delete_file_resendflag0 ",path);
			//打开文件一行行判断是否有flag!=0的  如果存在直接关闭文件并返回,如果不存在，删除文件
			fp = fopen(path, "r");
			if(fp)
			{
				while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
				{
					if(strlen(buff) > 18)
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') && (buff[strlen(buff)-20] == ','))
						{
							if(buff[strlen(buff)-2] != '0')			//检测是否存在resendflag != 0的记录   若存在则直接退出函数
							{
								flag = 1;
								break;
							}
						}
					}
					memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
				}
				fclose(fp);
				if(flag == 0)
				{
					print2msg(ECU_DBG_CONTROL_CLIENT,"unlink:",path);
					//遍历完文件都没发现flag != 0的记录直接删除文件
					unlink(path);
				}	
			}
		}
	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);
	return;

}

int change_inv_pro_result_flag(char *item,char flag)  //改变成功返回1，未找到该时间点返回0
{
	DIR *dirp;
	char dir[30] = "/home/data/iprocres";
	struct dirent *d;
	char path[100];
	char fileitem[4] = {'\0'};
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/iprocres");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"change_inv_pro_result_flag open directory error");
			mkdir("/home/data/iprocres",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			//打开文件一行行判断是否有flag=2的  如果存在直接关闭文件并返回1
			fp = fopen(path, "r+");
			if(fp)
			{
				while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
				{
					if(strlen(buff) > 18)
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') && (buff[strlen(buff)-20] == ',') )
						{
							if(buff[strlen(buff)-2] == '1')
							{
								memset(fileitem,0,4);
								memcpy(fileitem,&buff[strlen(buff)-6],3);				//获取每条记录的时间
								fileitem[3] = '\0';
								if(!memcmp(item,fileitem,4))						//每条记录的时间和传入的时间对比，若相同则变更flag				
								{
									fseek(fp,-2L,SEEK_CUR);
									fputc(flag,fp);
									//print2msg(ECU_DBG_CONTROL_CLIENT,"filetime",filetime);
									fclose(fp);
									free(buff);
									buff = NULL;
									list_destroy(&Head);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
				}
				fclose(fp);
			}
		}
	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);
	return 0;
	
}

//查询一条flag为1的数据   查询到了返回1  如果没查询到返回0
/*
data:表示获取到的数据
item：表示获取的命令帧
flag：表示是否还有下一条数据   存在下一条为1   不存在为0
*/
int search_pro_result_flag(char *data,char * item, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	int nextfileflag = 0;	
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(data,'\0',sizeof(data)/sizeof(data[0]));
	*flag = 0;
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		dirp = opendir("/home/data/proc_res");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"search_pro_result_flag open directory error");
			mkdir("/home/data/proc_res",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			if(tmp->date <= 20100101) 
			{
				unlink(path);
				continue;	//如果时间点小于2010 01 01 的话，跳过该文件
			}
			
			fp = fopen(path, "r");
			if(fp)
			{
				if(0 == nextfileflag)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(strlen(buff) > 18)	
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			
								{
									memcpy(item,&buff[strlen(buff)-6],3);				//获取每条记录的item
									memcpy(data,buff,(strlen(buff)-7));
									data[strlen(buff)-7] = '\n';
									//print2msg(ECU_DBG_CONTROL_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CONTROL_CLIENT,"search_readflag data",data);
									rt_hw_s_delay(1);
									while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	
									{
										if(strlen(buff) > 18)	
										{
											if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') )
											{
												if(buff[strlen(buff)-2] == sendflag)
												{
													*flag = 1;
													fclose(fp);
													free(buff);
													buff = NULL;
													list_destroy(&Head);
													rt_mutex_release(record_data_lock);
													return 1;
												}
											}
										}
										memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
									}
									nextfileflag = 1;
									break;
								}
							}
						}
						memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					}
				}else
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp)) 
					{
						if(strlen(buff) > 18)
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									free(buff);
									buff = NULL;
									list_destroy(&Head);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
						memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					}
				}
				
				fclose(fp);
			}
		}

	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);

	return nextfileflag;

}


//查询一条flag为1的数据   查询到了返回1  如果没查询到返回0
/*
data:表示获取到的数据
item：表示获取的命令帧
flag：表示是否还有下一条数据   存在下一条为1   不存在为0
*/
int search_inv_pro_result_flag(char *data,char * item,char *inverterid, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/data/iprocres";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	struct list *Head = NULL;
	struct list *tmp;
	char data_str[9] = {'\0'};
	int nextfileflag = 0;	
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(data,'\0',sizeof(data)/sizeof(data[0]));
	*flag = 0;
	Head = list_create(-1);
	if(result == RT_EOK)
	{
		dirp = opendir("/home/data/iprocres");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,"search_inv_pro_result_flag open directory error");
			mkdir("/home/data/iprocres",0);
		}
		else
		{
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memcpy(data_str,d->d_name,8);
				data_str[8] = '\0';
				list_addhead(Head,atoi(data_str)); 
			}
			closedir(dirp);
		}
		list_sort(Head);
		
		//读取数据
		for(tmp = Head->next; tmp != Head; tmp = tmp->next)
		{
			memset(path,0,100);
			memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
			sprintf(path,"%s/%d.dat",dir,tmp->date);
			if(tmp->date <= 20100101) 
			{
				unlink(path);
				continue;	//如果时间点小于2010 01 01 的话，跳过该文件
			}
			
			fp = fopen(path, "r");
			if(fp)
			{
				if(0 == nextfileflag)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  
					{
						if(strlen(buff) > 18)
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') && (buff[strlen(buff)-20] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			
								{
									memcpy(item,&buff[strlen(buff)-6],3);				//获取每条记录的item
									memcpy(inverterid,&buff[strlen(buff)-19],12);
									memcpy(data,buff,(strlen(buff)-20));
									data[strlen(buff)-20] = '\n';
									//print2msg(ECU_DBG_CONTROL_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CONTROL_CLIENT,"search_readflag data",data);
									rt_hw_s_delay(1);
									while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
									{
										if(strlen(buff) > 18)
										{
											if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
											{
												if(buff[strlen(buff)-2] == sendflag)
												{
													*flag = 1;
													fclose(fp);
													free(buff);
													buff = NULL;
													list_destroy(&Head);
													rt_mutex_release(record_data_lock);
													return 1;
												}
											}
										}
										memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
									}
									nextfileflag = 1;
									break;
									}
							}
						}	
						memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					}
				}else
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //?áè?ò?DDêy?Y
					{
						if(strlen(buff) > 18)
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-7] == ',') && (buff[strlen(buff)-20] == ',')  )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									free(buff);
									buff = NULL;
									list_destroy(&Head);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
						memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
					}
				}
				
				fclose(fp);
			}
		}
	}
	free(buff);
	buff = NULL;
	list_destroy(&Head);
	rt_mutex_release(record_data_lock);

	return nextfileflag;
	
}



/* 上报process_result表中的信息 */
int response_process_result()
{
	char *sendbuffer = NULL;
	int sockfd, flag;
	char inverterId[13] = {'\0'};
	//int item_num[32] = {0};
	char *data = NULL;//查询到的数据
	char item[4] = {'\0'};
	
	sendbuffer = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	data = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	memset(sendbuffer,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	memset(data,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);

	{
		//查询所有[ECU级别]处理结果
		//逐条上报ECU级别处理结果
		while(search_pro_result_flag(data,item,&flag,'1'))
		{
			printmsg(ECU_DBG_CONTROL_CLIENT,">>Start Response ECU Process Result");
			sockfd = Control_client_socket_init();
			if(sockfd < 0) 
			{
#ifdef WIFI_USE	

				//有线连接失败，使用wifi传输 
				{
					//发送一条记录
					if(SendToSocketC(control_client_arg.ip,randport(control_client_arg),data, strlen(data)) < 0){
						break;
					}
					//发送成功则将标志位置0
					change_pro_result_flag(item,'0');
					//WIFI_Close(SOCKET_C);
					printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
				}
#endif		

#ifndef WIFI_USE
		break;
#endif

			}else
			{
				//发送一条记录
				if(send_socket(sockfd, data, strlen(data)) < 0){
					close_socket(sockfd);
					continue;
				}
				//发送成功则将标志位置0
				change_pro_result_flag(item,'0');
				close_socket(sockfd);
				printmsg(ECU_DBG_CONTROL_CLIENT,">>End");
			}
			memset(data,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
		}				
		delete_pro_result_flag0();
		
		while(search_inv_pro_result_flag(data,item,inverterId,&flag,'1'))
		{
			char msg_length[6] = {'\0'};
			//拼接数据
			
			memset(sendbuffer, 0, sizeof(sendbuffer));
			sprintf(sendbuffer, "APS1300000A%03dAAA0%.12s%04d00000000000000END", atoi(item), ecu.ECUID12, 1);
			strcat(sendbuffer, data);

			if(sendbuffer[strlen(sendbuffer)-1] == '\n'){
				sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
			}
			else{
				sprintf(msg_length, "%05d", strlen(sendbuffer));
				strcat(sendbuffer, "\n");
			}
			strncpy(&sendbuffer[5], msg_length, 5);
			//发送数据
			printmsg(ECU_DBG_CONTROL_CLIENT,">>Start Response Inverter Process Result");
			sockfd = Control_client_socket_init();
			if(sockfd < 0)
			{
#ifdef WIFI_USE	

				//有线连接失败，使用wifi传输 
				{
					if(SendToSocketC(control_client_arg.ip,randport(control_client_arg),sendbuffer, strlen(sendbuffer)) < 0){
						break;
					}
					change_inv_pro_result_flag(item,'0');
				}
#endif	

#ifndef WIFI_USE
					break;
#endif
			}else
			{
				if(send_socket(sockfd, sendbuffer, strlen(sendbuffer)) < 0){
				close_socket(sockfd);
				continue;
				}
				change_inv_pro_result_flag(item,'0');
				close_socket(sockfd);
			}
			memset(data,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
		}
		delete_inv_pro_result_flag0();

	}
	free(sendbuffer);
	sendbuffer = NULL;
	free(data);
	data = NULL;
	return 0;
}


//该线程主要用于数据上传以及远程控制
void ECUControl_thread_entry(void* parameter)
{
	int ControlThistime=0, ControlDurabletime=65535, ControlReportinterval=900;
	int AlarmThistime=0, AlarmDurabletime=65535, AlarmReportinterval=180;
	char *data = NULL;
	int res,flag,result;
	FILE *fp;
	char time[15] = {'\0'};
	//添加功能函数
  	add_functions();
	printmsg(ECU_DBG_CONTROL_CLIENT,"add functions Over");
	rt_thread_delay(RT_TICK_PER_SECOND*START_TIME_CONTROL_CLIENT);
	
	data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
	memset(data,0x00,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);

	while(1)
	{
		//远程控制上报
		if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
		{	
			//远程控制 15分钟上报	
			printmsg(ECU_DBG_CONTROL_CLIENT,"Control DATA Start");
			ControlDurabletime = acquire_time();
			ControlThistime = acquire_time();
			
			if(22 == get_hour())
			{
				precontrolprocess();
				resend_control_record();
				delete_control_file_resendflag0();		//清空数据resend标志全部为0的目录
			}
			memset(data,0x00,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
			while(search_control_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				if(compareTime(ControlDurabletime ,ControlThistime,ControlReportinterval))
				{
						break;
				}
				//printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_control_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			delete_control_file_resendflag0();		//清空数据resend标志全部为0的目录
			fp=fopen("/TMP/ECUUPVER.CON","r");
			if(fp!=NULL)
			{
				char c='0';
				c=fgetc(fp);
				fclose(fp);
				if(c=='1')
				{
					result = communication_with_EMA(102);
					if(result != -1)
					{
						unlink("/TMP/ECUUPVER.CON");
					}
					
				}
			}else
			{
				communication_with_EMA(0);
			}
			

			printmsg(ECU_DBG_CONTROL_CLIENT,"Control DATA End");

		}
		
		
		//上报告警标志
		if(compareTime(AlarmDurabletime ,AlarmThistime,AlarmReportinterval))
		{			
			printmsg(ECU_DBG_CONTROL_CLIENT,"Alarm DATA Start");
			AlarmDurabletime = acquire_time();
			AlarmThistime = acquire_time();

			if(23 == get_hour())
			{
				//发送A157 情况异常标志命令
				prealarmprocess();
				//重发标志位为2的标志
				resend_alarm_record();
				delete_alarm_file_resendflag0();		//清空数据resend标志全部为0的目录
			}
			response_process_result();
			memset(data,0x00,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
			while(search_alarm_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				
				if(compareTime(AlarmDurabletime ,AlarmThistime,AlarmReportinterval))
				{
						break;
				}
				//printmsg(ECU_DBG_CONTROL_CLIENT,data);
				res = send_alarm_record( data, time);
				if(-1 == res)
					break;
				memset(data,0,CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
				memset(time,0,15);
			}
			
			delete_alarm_file_resendflag0();		//清空数据resend标志全部为0的目录
			printmsg(ECU_DBG_CONTROL_CLIENT,"Alarm DATA End");
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND);	
		ControlDurabletime = acquire_time();		
		AlarmDurabletime = acquire_time();

	}
	
}


#ifdef RT_USING_FINSH
#include <finsh.h>
void commEMA(void)
{
	communication_with_EMA(0);
}

void procResult(void)
{
	response_process_result();
}
FINSH_FUNCTION_EXPORT(commEMA, eg:commEMA());
FINSH_FUNCTION_EXPORT(procResult, eg:procResult());
#endif

