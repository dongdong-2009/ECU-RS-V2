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
#include "serverfile.h"
#include "timer.h"


/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern struct rt_device serial4;		//串口4为zigbee收发串口
extern ecu_info ecu;
extern int zigbeeReadFlag;
static int zigbeereadtimeoutflag = 0;
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
rt_mutex_t heart_lock = RT_NULL;


const unsigned short CRC_table_16[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

#define ZIGBEE_SERIAL (serial4)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//CRC校验，用于FBFB后的数据校验
unsigned short GetCrc_16(unsigned char * pData, unsigned short nLength, unsigned short init, const unsigned short *ptable)
{
  unsigned short cRc_16 = init;
  unsigned char  temp;

  while(nLength-- > 0)
  {
    temp = cRc_16 >> 8; 
    cRc_16 = (cRc_16 << 8) ^ ptable[(temp ^ *pData++) & 0xFF];
  }

  return cRc_16;
}

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
			rt_thread_delay(1);
			if(zigbeeReadFlag == 1)	//串口数据监测,如果有数据则返回1
			{
				rt_timer_delete(readtimer);
				rt_thread_delay(8);
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
	rt_thread_delay(1);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_thread_delay(1);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_thread_delay(1);

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
	if(heart_lock == RT_NULL)
	{
		heart_lock = rt_mutex_create("heart_lock", RT_IPC_FLAG_FIFO);
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
	rt_thread_delay(100);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//设置引脚为高电平输出

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
	rt_thread_delay(RT_TICK_PER_SECOND*2);
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
	rt_thread_delay(30); 

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
		rt_thread_delay(20);
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
		printhexmsg(ECU_DBG_COMM,"Send", (char *)sendbuff, length+15);
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
	printhexmsg(ECU_DBG_COMM,"Send", (char *)sendbuff, length+15);
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

	if(selectZigbee(120) <= 0)
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
		printhexmsg(ECU_DBG_COMM,"Reply", data_all, temp_size);
		rt_thread_delay(90);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(0==rt_strcmp(inverter->uid,inverterid)))
		{
			ecu.Signal_Level = data_all[4];
			inverter->RSSI = data_all[4];
			//如果获取到的短地址和原来的短地址不相同，更新短地址
			if((data_all[2]*256 + data_all[3]) != inverter->shortaddr)
			{
				inverter->shortaddr= data_all[2]*256 + data_all[3];
				updateID();
			}
			return size;
		}
		else
		{
			inverter->RSSI = 0;
			return -1;
		}
	}

}

int zb_send_heart_cmd(char *buff, int length)		//zigbee包头
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
	unsigned short crc16 = 0;
	print2msg(ECU_DBG_COMM,"Query inverter data",inverter->uid);
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
	rt_mutex_take(heart_lock, RT_WAITING_FOREVER);
	zb_send_heart_cmd(sendbuff, i);
	COMM_Timeout_Event = 0;
	ret = zb_get_heart_reply(data,inverter);
	rt_mutex_release(heart_lock);

	if((ret != 0)&&(ret%74 == 0)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[72])&&(0xFE == data[73]))
	{
		crc16 = GetCrc_16((unsigned char *)&data[2],68,0,CRC_table_16);
		//printf("%02x %02x\n",crc16/256,crc16%256);
		if((data[70] == crc16/256)&&(data[71] == crc16%256))
		{
			if(resolvedata_OPT700_RS(&data[4], inverter) == -1)
				return -1;
			else
				return 1;
		}else
		{
			return -1;
		}

	}
	else
	{
		return -1;
	}

}

int zb_sendHeart(char uid[13])
{
	int i=0;
	char sendbuff[256];
	//print2msg(ECU_DBG_COMM,"Query XXX inverter data",uid);
	
	//sendbuff[i++] = (uid[0] - '0') * 0x10 + (uid[1] - '0');
	//sendbuff[i++] = (uid[2] - '0') * 0x10 + (uid[3] - '0');
	//sendbuff[i++] = (uid[4] - '0') * 0x10 + (uid[5] - '0');
	//sendbuff[i++] = (uid[6] - '0') * 0x10 + (uid[7] - '0');
	//sendbuff[i++] = (uid[8] - '0') * 0x10 + (uid[9] - '0');
	//sendbuff[i++] = (uid[10] - '0') * 0x10 + (uid[11] - '0');
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
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
	rt_mutex_take(heart_lock, RT_WAITING_FOREVER);
	zb_send_heart_cmd(sendbuff, i);
	rt_mutex_release(heart_lock);
	return 0;
		
}

/*
functionStatus:0不改变 1使能 2禁能
onoff: 0开机 1关机 2不改变
RSDTimeout:RSD超时时间 0-255 S
*/
//设置心跳状态开关
int zb_set_heartSwitch_boardcast(unsigned char functionStatus,unsigned char onoff,unsigned char RSDTimeout)
{
	int i=0;
	char sendbuff[256];
	unsigned short crc16 = 0;
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x02;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	if(functionStatus == 0x02)			//RSD功能状态禁能
	{
		sendbuff[i++] = 0x02;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","functionStatus : 2");
	}else if(functionStatus == 0x01)	//RSD功能状态使能
	{
		sendbuff[i++] = 0x01;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","functionStatus : 1");
	}else								//RSD功能状态不改变
	{
		sendbuff[i++] = 0x00;
	}

	if(onoff == 0x00)				//开关机状态打开
	{
		sendbuff[i++] = 0x00;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","onoff : 0");
	}else							//关机或者不改变	
	{
		sendbuff[i++] = 0x01;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_boardcast","onoff : 1");
	}



	sendbuff[i++] = RSDTimeout;
	
	//校验值
	crc16 = GetCrc_16((unsigned char *)&sendbuff[2],9,0,CRC_table_16);
	sendbuff[i++] = crc16/256;
	sendbuff[i++] = crc16%256;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	zb_broadcast_cmd(sendbuff, i);
	return 0;

}


int zb_set_heartSwitch_single(inverter_info *inverter,unsigned char functionStatus,unsigned char onoff,unsigned char RSDTimeout)
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	unsigned short crc16 = 0;
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x02;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCD;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	
	if(functionStatus == 0x02)			//RSD功能状态禁能
	{
		sendbuff[i++] = 0x02;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_single","functionStatus : 2");
	}else if(functionStatus == 0x01)	//RSD功能状态使能
	{
		sendbuff[i++] = 0x01;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_single","functionStatus : 1");
	}else								//RSD功能状态不改变
	{
		sendbuff[i++] = 0x00;
	}

	if(onoff == 0x00)				//开关机状态打开
	{
		sendbuff[i++] = 0x00;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_single","onoff : 0");
	}else							//关机或者不改变	
	{
		sendbuff[i++] = 0x01;
		print2msg(ECU_DBG_COMM,"zb_set_heartSwitch_single","onoff : 1");
	}

	sendbuff[i++] = RSDTimeout;
	
	
	//校验值
	crc16 = GetCrc_16((unsigned char *)&sendbuff[2],9,0,CRC_table_16);
	sendbuff[i++] = crc16/256;
	sendbuff[i++] = crc16%256;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);

	if((15 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[13])&&(0xFE == data[14]))
	{
		crc16 = GetCrc_16((unsigned char *)&data[2],9,0,CRC_table_16);
		if((data[11] == crc16/256)&&(data[12] == crc16%256))
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
		}else
		{
			return -1;
		}

	}
	else
	{
		return -1;
	}
}

