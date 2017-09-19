/*****************************************************************************/
/* File      : remote_control_protocol.c                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define VERSION 13

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 生成协议头 */
int msg_Header(char *sendbuffer, const char *cmd_id)
{
	memset(sendbuffer, 0, sizeof(sendbuffer));
	snprintf(sendbuffer, 18+1, "APS%02d00000%.4sAAA0", VERSION, cmd_id);
	return 0;
}

/*
 * ECU向EMA请求消息
 *
 * 输入：待发送缓存指针：sendbuffer
 * 		ECU号码：ecuid
 * */
int msg_REQ(char *sendbuffer)
{
	char msg_length[6] = {'\0'};
	msg_Header(sendbuffer, "A101");
	strcat(sendbuffer, ecu.ECUID12);
	strcat(sendbuffer, "A10100000000000000END\n");
	
	sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	strncpy(&sendbuffer[5], msg_length, 5);
		
	return 0;
}

/*
 * ECU向EMA应答消息
 *
 * 输入：待发送缓存指针：sendbuffer
 * 		ECU号码：ecuid
 * 		协议命令号：cmd_id
 * 		协议时间戳：timestamp
 * 		应答标志位：ack_flag
 * */
int msg_ACK(char *sendbuffer,
		const char *cmd_id, const char *timestamp, int ack_flag)
{
	char msg_body[35] = {'\0'};
	char msg_length[6] = {'\0'};

	msg_Header(sendbuffer, "A100");
	sprintf(msg_body, "%.12s%.4s%.14s%1dEND\n", ecu.ECUID12, cmd_id, timestamp, ack_flag);
	strcat(sendbuffer, msg_body);
	sprintf(msg_length, "%05d", strlen(sendbuffer)-1);
	strncpy(&sendbuffer[5], msg_length, 5);
	
	return 0;
}

/*
 *  从协议中解析出整型变量(兼容以‘A’填充的整数)
 *
 *  输入：整型变量在字符串中的起始指针：s
 *  	 整型变量所占字符串的长度：len *
 *  输出：整型变量
 */
int msg_get_int(const char *s, int len)
{
	int i, count = 0;
	char buffer[16] = {'\0'};

	strncpy(buffer, s, len);
	for(i=0; i<len; i++)
	{
		if('A' == buffer[i]){
			buffer[i] = '0';
			count++;
		}
	}
	if(count >= len)
		return -1;
	else
		return atoi(buffer);
}

/* 获取协议信息版本号 */
int msg_version(const char *msg)
{
	return msg_get_int(&msg[3], 2);
}

/* 协议信息长度 */
int msg_length(const char *msg)
{
	return msg_get_int(&msg[5], 5);
}

/* 获取协议命令号 */
int msg_cmd_id(const char *msg)
{
	int cmd_id;

	cmd_id = msg_get_int(&msg[10], 4);
	if(cmd_id == 101){
		return msg_get_int(&msg[30], 4);
	}
	return cmd_id;
}

/* 获取协议消息流水号 */
int msg_seq_id(const char *msg)
{
	return msg_get_int(&msg[14], 4);
}

/*
 * 	检查每条信息通用部分的格式
 *
 * 	输入：信息字符串指针：msg
 * 	输出：正确：0 , 错误：-1
 * */
int msg_format_check(const char *msg)
{
	//协议头APS
	if(strncmp(msg, "APS", 3)){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: APS");
		return -1;
	}

	//版本号
	//msg_version(msg);

	//协议长度
	if(msg_length(msg) != strlen(msg)){
		printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: length");
		return -1;
	}

	//ECU_ID
	if(strncmp(&msg[18], ecu.ECUID12, 12)){
		if(msg_get_int(&msg[10], 4) != 123){ //A123应答没有ECU_ID
			printmsg(ECU_DBG_CONTROL_CLIENT,"Format Error: ecu_id");
			return -1;
		}
	}

	//协议尾END

	return 0;
}

/*
 * 	检查逆变器信息条数是否与ECU信息中的NUM变量相同
 *
 * 	输入：逆变器信息起始点的字符串指针：s
 * 		 ECU信息中的NUM变量：num
 * 		 每条逆变器信息的长度(不包括END)：len
 *		 是否每条逆变器信息都以END结尾：flag
 * 	输出：正确：true , 错误：false
 * */
int msg_num_check(const char *s, int num, int len, int flag)
{
	if(flag){
		return (strlen(s) == (num*(len+3)));
	}
	else{
		return (strlen(s) == (num*len + 3));
	}
}

/* 将字符串连接到协议字符串 */
char *msgcat_s(char *s, int size, const char *value)
{
	int i, length;

	length = strlen(value);
	if(size < length){
		strncat(s, value, size);
		return s;
	}
	for(i=size; i>length; i--){
		strcat(s, "0");
	}
	strcat(s, value);
	return s;
}

/* 将整型数据连接到协议字符串 */
char *msgcat_d(char *s, int size, int value)
{
	int i, length;
	char buffer[32] = {'\0'};

	if(value < 0){
		for(i=0; i<size; i++)
			strcat(s, "A");
		return s;
	}
	sprintf(buffer, "%d", value);
	length = strlen(buffer);
	if(size < length){
		strncat(s, buffer, size);
		return s;
	}
	for(i=size; i>length; i--){
		strcat(s, "0");
	}
	strcat(s, buffer);
	return s;
}

/* 从给定字符串中复制出一段到END为止的字符串 */
int msg_get_one_section(char *s, const char *msg)
{
	memset(s, 0, sizeof(s));
	if(strstr(msg, "END") == NULL){
		return -1;
	}
	strncpy(s, msg, (int)(strstr(msg, "END")-msg));
	return strlen(s);
}

