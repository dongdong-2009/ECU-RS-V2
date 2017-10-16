/*****************************************************************************/
/*  File      : zigbee.c                                                     */
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
#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include "debug.h"
#include "zigbee.h"
#include <rthw.h>
#include <stm32f10x.h>
#include <dfs_posix.h> 
#include <stdio.h>
#include "rthw.h"
#include "resolve.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern struct rt_device serial4;		//串口4为zigbee收发串口
extern ecu_info ecu;
extern int zigbeeReadFlag;
static int zigbeereadtimeoutflag = 0;
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

#define ZIGBEE_SERIAL (serial4)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//定时器超时函数
static void readtimeout_Zigbee(void* parameter)
{
	zigbeereadtimeoutflag = 1;
}

int selectZigbee(int timeout)			// 参数为1表示10ms zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{
	
	rt_timer_t readtimer;
	readtimer = rt_timer_create("read", /* 定时器名字为 read */
					readtimeout_Zigbee, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND/100, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
	zigbeereadtimeoutflag = 0;

	while(1)
	{
		if(zigbeereadtimeoutflag)
		{
			rt_timer_delete(readtimer);
			return 0;
		}else 
		{
			rt_hw_ms_delay(10);
			if(zigbeeReadFlag == 1)	//串口数据监测,如果有数据则返回1
			{
				rt_timer_delete(readtimer);
				rt_hw_ms_delay(80);
				return 1;
			}
		}
	}
}

void clear_zbmodem(void)		//清空串口缓冲区的数据
{
	char data[256];
	//清空缓冲器代码	通过将接收缓冲区的所有数据都读取出来，从而清空数据
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(10);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(10);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(10);

}

int openzigbee(void)
{
	int result = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	rt_device_t new;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//设置引脚为高电平输出，使能Zigbbe模块
	
	new = rt_device_find("uart4");		//寻找zigbee串口并配置模式
	if (new != RT_NULL)
	{
		result = rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
		if(result)
		{
			printdecmsg(ECU_DBG_COMM,"open Zigbee failed ",result);
		}else
		{
			printmsg(ECU_DBG_COMM,"open Zigbee success");
		}
	}

	return result;
}

//复位zigbee模块  通过PC7的电平置高置低然后达到复位的效果
void zigbee_reset(void)
{
	//先设置PC7为低电平，然后再设置为高电平达到复位的功能
	GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);		//设置引脚为低电平输出
	rt_hw_ms_delay(1000);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//设置引脚为高电平输出
	rt_hw_s_delay(10);
	printmsg(ECU_DBG_COMM,"zigbee reset successful");
}

int zb_get_reply_from_module(char *data)			//读取zigbee模块的返回帧
{
	int size = 0;

	if(selectZigbee(100) <= 0)
	{
		printmsg(ECU_DBG_COMM,"Get reply time out");
		return -1;
	}else
	{
		size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
		if(size > 0)
		{
			printhexmsg(ECU_DBG_COMM,"Reply", data, size);
			return size;
		}else
		{
			return -1;
		}
	}	
}


int zb_test_communication(void)		//zigbee测试通信有没有断开
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret = 0;
	char data[256] =  {'\0'};
	int check=0;

	printmsg(ECU_DBG_COMM,"test zigbee communication");
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0D;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;

}

//设置ECU的PANID和信道
int zb_change_ecu_panid(void)
{
	unsigned char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg(ECU_DBG_COMM,"Set ECU PANID and Channel", (char *)sendbuff, 15);

	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_hw_s_delay(2); //延时2S，因为设置完ECU信道和PANID后会发6个FF
		return 1;
	}

	return -1;
}

//设置ECU的PANID为0xFFFF,信道为指定信道(注:向逆变器发送设置命令时,需将ECU的PANID设为0xFFFF)
int zb_restore_ecu_panid_0xffff(int channel)
{
	unsigned char sendbuff[15] = {'\0'};
	char recvbuff[256];
	int i;
	int check=0;
	//向ECU发送命令
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0xFF;
	sendbuff[8]  = 0xFF;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	printhexmsg(ECU_DBG_COMM,"Change ECU channel (PANID:0xFFFF)", (char *)sendbuff, 15);

	//接收反馈
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		return 1;
	}

	return -1;
}

//单点改变逆变器的PANID为ECU的MAC地址后四位,信道为指定信道(注:需要将ECU的PANID改为0xFFFF(万能发送))
int zb_change_inverter_channel_one(char *inverter_id, int channel)
{
	char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	rt_hw_ms_delay(300); 

	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter_id[0]-0x30)*16+(inverter_id[1]-0x30));
	sendbuff[16]=((inverter_id[2]-0x30)*16+(inverter_id[3]-0x30));
	sendbuff[17]=((inverter_id[4]-0x30)*16+(inverter_id[5]-0x30));
	sendbuff[18]=((inverter_id[6]-0x30)*16+(inverter_id[7]-0x30));
	sendbuff[19]=((inverter_id[8]-0x30)*16+(inverter_id[9]-0x30));
	sendbuff[20]=((inverter_id[10]-0x30)*16+(inverter_id[11]-0x30));
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 21);
	printhexmsg(ECU_DBG_COMM,"Change Inverter Channel (one)", sendbuff, 21);

	rt_hw_s_delay(1); //此处延时必须大于1S

	return 0;
}


//关闭逆变器ID上报 + 绑定逆变器
int zb_off_report_id_and_bind(int short_addr)
{
	int times = 3;
	char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;

	do {
		rt_hw_ms_delay(200);
		//发送关闭逆变器ID上报+绑定操作
		clear_zbmodem();
		sendbuff[0]  = 0xAA;
		sendbuff[1]  = 0xAA;
		sendbuff[2]  = 0xAA;
		sendbuff[3]  = 0xAA;
		sendbuff[4]  = 0x08;
		sendbuff[5]  = short_addr>>8;
		sendbuff[6]  = short_addr;
		sendbuff[7]  = 0x00; //PANID(逆变器不解析)
		sendbuff[8]  = 0x00; //PANID(逆变器不解析)
		sendbuff[9]  = 0x00; //信道(逆变器不解析)
		sendbuff[10] = 0x00; //功率(逆变器不解析)
		sendbuff[11] = 0xA0;
		for(i=4;i<12;i++)
			check=check+sendbuff[i];
		sendbuff[12] = check/256;
		sendbuff[13] = check%256;
		sendbuff[14] = 0x00;
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
		printhexmsg(ECU_DBG_COMM,"Bind ZigBee", sendbuff, 15);

		//接收逆变器应答(短地址,ZigBee版本号,信号强度)
		if ((11 == zb_get_reply_from_module(recvbuff))
				&& (0xA5 == recvbuff[2])
				&& (0xA5 == recvbuff[3])) {
			//update_turned_off_rpt_flag(short_addr, (int)recvbuff[9]);
			//update_bind_zigbee_flag(short_addr);
			printmsg(ECU_DBG_COMM,"Bind Successfully");
			return 1;
		}
	}while(--times);

	return 0;
}


int zigbeeRecvMsg(char *data, int timeout_sec)
{
	int count;
	if (selectZigbee(timeout_sec*100) <= 0) {
		printmsg(ECU_DBG_COMM,"Get reply time out");
		return -1;
	} else {
		
		count = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
		printhexmsg(ECU_DBG_COMM,"Reply", data, count);
		return count;
	}
}

int zb_send_cmd(inverter_info *inverter, char *buff, int length)		//zigbee包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = inverter->shortaddr>>8;
	sendbuff[6]  = inverter->shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	printdecmsg(ECU_DBG_COMM,"shortaddr",inverter->shortaddr);
	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	if(0!=inverter->shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);
		//printhexmsg("Send", (char *)sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_broadcast_cmd(char *buff, int length)		//zigbee广播包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);

	return 1;
}

int zb_get_reply(char *data,inverter_info *inverter)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	if(selectZigbee(100) <= 0)
	{
		printmsg(ECU_DBG_COMM,"Get reply time out");
		inverter->RSSI=0;
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg(ECU_DBG_COMM,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(0==rt_strcmp(inverter->uid,inverterid)))
		{
			ecu.Signal_Level = data_all[4];
			inverter->RSSI = data_all[4];
			return size;
		}
		else
		{
			inverter->RSSI = 0;
			return -1;
		}
	}

}

int zb_get_heart_reply(char *data,inverter_info *inverter)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	if(selectZigbee(100) <= 0)
	{
		//printmsg(ECU_DBG_COMM,"Get reply time out");
		inverter->RSSI=0;
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		//printhexmsg(ECU_DBG_COMM,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(0==rt_strcmp(inverter->uid,inverterid)))
		{
			ecu.Signal_Level = data_all[4];
			inverter->RSSI = data_all[4];
			return size;
		}
		else
		{
			inverter->RSSI = 0;
			return -1;
		}
	}

}

int zb_send_heart_cmd(inverter_info *inverter, char *buff, int length)		//zigbee包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x66;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}
/*
	for(i= 0;i<length+15;i++)
		printf("%02x ",sendbuff[i]);
	printf("\n");
*/
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);
	return 1;

}

int zb_query_heart_data(inverter_info *inverter)		//请求逆变器实时数据
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];

	//print2msg(ECU_DBG_COMM,"Query inverter data",inverter->uid);
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = (inverter->uid[0] - '0') * 0x10 + (inverter->uid[1] - '0');
	sendbuff[i++] = (inverter->uid[2] - '0') * 0x10 + (inverter->uid[3] - '0');
	sendbuff[i++] = (inverter->uid[4] - '0') * 0x10 + (inverter->uid[5] - '0');
	sendbuff[i++] = (inverter->uid[6] - '0') * 0x10 + (inverter->uid[7] - '0');
	sendbuff[i++] = (inverter->uid[8] - '0') * 0x10 + (inverter->uid[9] - '0');
	sendbuff[i++] = (inverter->uid[10] - '0') * 0x10 + (inverter->uid[11] - '0');
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x02;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xBB;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	//校验值
	sendbuff[i++] = 0xB2;
	sendbuff[i++] = 0xCE;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_heart_cmd(inverter, sendbuff, i);
	ret = zb_get_heart_reply(data,inverter);

	if((74 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[72])&&(0xFE == data[73]))
	{
		inverter->status.dataflag = 1;	//接收到数据置为1
		resolvedata_OPT700_RS(&data[4], inverter);
		
		return 1;
	}
	else
	{
		inverter->status.dataflag = 0;		//没有接收到数据就置为0
		return -1;
	}

}


//设置心跳状态开关
int zb_set_heartSwitch_boardcast(unsigned char functionStatus)
{
	int i=0;
	char sendbuff[256];
	
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x02;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	if(functionStatus == 0x00)
	{
		sendbuff[i++] = 0x02;
		sendbuff[i++] = 0x00;
		sendbuff[i++] = 0x00;
		//校验值
		sendbuff[i++] = 0xFC;
		sendbuff[i++] = 0xA0;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","2");
	}else
	{
		sendbuff[i++] = 0x01;
		sendbuff[i++] = 0x00;
		sendbuff[i++] = 0x00;
		//校验值
		sendbuff[i++] = 0xA5;
		sendbuff[i++] = 0xF0;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","1");
	}
	
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	zb_broadcast_cmd(sendbuff, i);
	return 0;

}


int zb_set_heartSwitch_single(inverter_info *inverter,unsigned char functionStatus)
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x02;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC6;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	
	if(functionStatus == 0x00)
	{
		sendbuff[i++] = 0x02;
		sendbuff[i++] = 0x00;
		sendbuff[i++] = 0x00;
		//校验值
		sendbuff[i++] = 0x32;
		sendbuff[i++] = 0x40;
	}else
	{
		sendbuff[i++] = 0x01;
		sendbuff[i++] = 0x00;
		sendbuff[i++] = 0x00;
		//校验值
		sendbuff[i++] = 0x6B;
		sendbuff[i++] = 0x10;
	}

	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);

	if((15 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[13])&&(0xFE == data[14]))
	{
		if(data[5] == 0xDE)
		{
			print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_single success",inverter->uid);
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

