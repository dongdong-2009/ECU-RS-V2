#include "ZigBeeChannel.h"
#include "remote_control_protocol.h"
#include "channel.h"
#include "string.h"
#include "variation.h"
#include "Serverfile.h"
#include "stdio.h"
#include "threadlist.h"
#include "stdlib.h"
#include "dfs_posix.h"
#include "AppFunction.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


/* 判断是否需要改变信道 */
int ZigbeeChannel_need_report()
{
	FILE *fp;
	char buff[2] = {'\0'};

	fp = fopen("/tmp/repChan.flg", "r");
	if (fp) {
		fgets(buff, 2, fp);
		fclose(fp);
	}

	return ('1' == buff[0]);
}

void ResponseECUZigbeeChannel(char channel,unsigned short panid,unsigned short shortadd)
{
	char *inverter_result = NULL;
	
	//查看/tmp/repChan.flag文件是否为1 
	if(ZigbeeChannel_need_report())
	{
		inverter_result = malloc(200);
    		memset(inverter_result,'\0',200);
		sprintf(inverter_result, "000000000000%02x%04x%05dEND", channel,panid,shortadd);
		save_inverter_parameters_result2("000000000000",172,inverter_result);
		free(inverter_result);
		inverter_result = NULL;
		unlink("/tmp/repChan.flg");
	}
}

void ResponseZigbeeChannel(char *UID,char channel,unsigned short panid,unsigned short shortadd)
{
	char *inverter_result = malloc(200);
	memset(inverter_result,'\0',200);
	sprintf(inverter_result, "%s%02x%04x%05dEND", UID,channel,panid,shortadd);
	save_inverter_parameters_result2(UID,172,inverter_result);
	free(inverter_result);
	inverter_result = NULL;
}

int ecu_ZigBeeChannel_msg(char *sendbuffer, int num, const char *recvbuffer)
{

	char timestamp[15] = {'\0'};	//时间戳

	strncpy(timestamp, &recvbuffer[34], 14);
	msgcat_s(sendbuffer, 12, ecu.ECUID12);
	msgcat_d(sendbuffer, 4, num);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}

int inverter_ZigBeeChannel_msg(char *sendbuffer, char* id,char channel,unsigned short panID,unsigned short shortAdd)
{
	char temp[6] = {'\0'};
	strcat(sendbuffer, id); //inverter UID
	sprintf(temp,"%02x",channel);
	temp[2] = '\0';
	strcat(sendbuffer, temp); 	 //channel
	
	sprintf(temp,"%04x",panID);
	temp[4] = '\0';
	strcat(sendbuffer, temp); 	 //panID
	
	sprintf(temp,"%05d",shortAdd);
	temp[5] = '\0';
	strcat(sendbuffer, temp); 	 //shortAdd
	
	strcat(sendbuffer, "END"); 	 //end

	return 0;
}

int SetChannel_All(const char *recvbuffer)
{
	unsigned char oldChannel = 0,newChannel = 0;
	oldChannel = strtohex((char *)&recvbuffer[48]);
	newChannel = strtohex((char *)&recvbuffer[50]);
	printf("SetChannel_All oldChannel:%02x,newChannel:%02x\n",oldChannel,newChannel);
	if((oldChannel != 0x00) &&(oldChannel<0x0B)&&(oldChannel>0x1A)&&(newChannel<0x0B)&&(newChannel>0x1A))
	{	//传入数据错误
		return 1;
	}else
	{
		saveOldChannel(oldChannel);
		saveNewChannel(newChannel);
		saveChannel_change_flag();
		echo ("/tmp/repChan.flg","1");
		//重启main线程
                  threadRestartTimer(10,TYPE_DATACOLLECT);

		return 0;

	}
}

int SetChannel_Single(const char *recvbuffer)
{
	return 1;
}

int SetChannel_ECU(const char *recvbuffer)
{
	unsigned char oldChannel = 0,newChannel = 0;
	char temp[5] = {'\0'};
	oldChannel = strtohex((char *)&recvbuffer[48]);
	newChannel = strtohex((char *)&recvbuffer[50]);
	printf("SetChannel_ECU oldChannel:%02x,newChannel:%02x\n",oldChannel,newChannel);
	if((oldChannel != 0x00) &&(oldChannel<0x0B)&&(oldChannel>0x1A)&&(newChannel<0x0B)&&(newChannel>0x1A))
	{	//传入数据错误
		return 1;
	}else
	{
		sprintf(temp,"0x%02x",newChannel);
		temp[4] = '\0';
		echo("/config/channel.con",temp);
		echo ("/tmp/repChan.flg","1");	//上报当前信道标志更改为1
		//重启main线程
                  threadRestartTimer(10,TYPE_DATACOLLECT);

		return 0;
	}
}

/* [A171] EMA Set inverter Channel */
int set_ZigBeeChannel(const char *recvbuffer, char *sendbuffer)
{
	int flag;
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};

	//获取设置类型标志
	sscanf(&recvbuffer[30], "%1d", &flag);	//标志位 0:全部设置  1:部分设置 2:设置ECU信道
	//获取时间戳
	strncpy(timestamp, &recvbuffer[31], 14);
	switch(flag)
	{
		case 0:	//设置所有信道
			if(SetChannel_All(recvbuffer))
				ack_flag = DB_ERROR;
			break;
		case 1:	//设置部分信道
			if(SetChannel_Single(recvbuffer))
				ack_flag = DB_ERROR;
			break;
		case 2:	//设置ECU信道
			if(SetChannel_ECU(recvbuffer))
				ack_flag = DB_ERROR;
			break;
		default:
			ack_flag = FORMAT_ERROR; //格式错误
			break;
	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A171", timestamp, ack_flag);
	return 0; 
}

/* [A172] ECU Response Zigbee Channel */
int response_ZigBeeChannel_Result(const char *recvbuffer, char *sendbuffer)
{
	int i;

	/* Head Info*/
	strcpy(sendbuffer, "APS13AAAAAA172AAA0"); 
	{
		/* ECU Message */
		ecu_ZigBeeChannel_msg(sendbuffer, (ecu.validNum + 1), recvbuffer);
		//拼接ECU信道UID全部写0
		inverter_ZigBeeChannel_msg(sendbuffer,"000000000000",ecu.channel,ecu.panid,0);
	
		for(i = 0; i < ecu.validNum;i++)
		{
			inverter_ZigBeeChannel_msg(sendbuffer,inverterInfo[i].uid,0,0,inverterInfo[i].shortaddr);		
		}
	}
	return 0;
}

