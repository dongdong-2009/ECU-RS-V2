#include "ZigBeeTransmission.h"
#include "string.h"
#include "stdlib.h"
#include "remote_control_protocol.h"
#include "Serverfile.h"
#include "stdio.h"
#include "zigbee.h"
#include "AppFunction.h"
#include "dfs_posix.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];

int formatTransmission(const char *recvbuffer)	//接受格式判断
{
	int i=0;
	unsigned short packLen = 0,uidNum = 0,len=0;
	char temp[6] = {'\0'};
	memcpy(temp,&recvbuffer[5],5);
	for(i=0;i<5;i++)
	{
		if(temp[i] == 'A')
			temp[i] = '0';
	}
	temp[5] = '\0';
	len = atoi(temp);
	memcpy(temp,&recvbuffer[52],3);
	temp[3] = '\0';
	packLen = atoi(temp);
	memcpy(temp,&recvbuffer[55+packLen],3);
	temp[3] = '\0';
	uidNum = atoi(temp);

	//判断长度是否正确

	if(len == (61+packLen+12*uidNum))
		return 0;	//检查成功
	else 
		return 1;	//检测失败
}

void ResponseZigbeeTransimission(int flag,char *UID,unsigned short packlen,unsigned char *buff)
{
	int i = 0,curlen;
	char *inverter_result = malloc(600);
	memset(inverter_result,'\0',600);
	if(flag == 1)
	{
		sprintf(inverter_result, "%s%03d", UID,packlen);
		curlen = 15;
		for(i=0;i<(packlen/2);i++)
		{
			sprintf(&inverter_result[curlen],"%02x",buff[i]);
			curlen += 2;
		}
	}else
	{
		sprintf(inverter_result, "%s007TimeOutEND", UID);
	}

	inverter_result[15+packlen] = 'E';
	inverter_result[16+packlen] = 'N';
	inverter_result[17+packlen] = 'D';
	inverter_result[18+packlen] = '\0';
	
	save_inverter_parameters_result2(UID,174,inverter_result);
	free(inverter_result);
	inverter_result = NULL;
}

void ZigBeeTransimission(int flag,unsigned short sendNum,unsigned short TimeInterval
	,unsigned short packlen,char *packet,unsigned short uidNum,char *Uidlist)
{
	int i = 0,j = 0,k;
	unsigned char buff[256] = {'\0'};//需要发送的报文
	char data[256] = {'\0'};
	int ret = 0;
	inverter_info *curinverter = inverterInfo;
	char UID[12];
  if(-1 == strToHex(packet,buff,packlen/2))	return;
		 
	switch(flag)
	{
		case 0:	//广播0x55
			for( i = 0;i<sendNum;i++)
			{
				zb_broadcast_cmd((char *)buff,packlen/2);
				rt_thread_delay(RT_TICK_PER_SECOND*TimeInterval);
			}
			ResponseZigbeeTransimission(1,"000000000000",packlen,buff);
			break;
		case 1:	//单播0x55
			for(i = 0;i<uidNum;i++)
			{
				memcpy(UID,&Uidlist[0+12*i],12);
				curinverter = inverterInfo;
				for(j=0; j<MAXINVERTERCOUNT; j++, curinverter++)
				{
					if(!memcmp(curinverter->uid,UID,12))
					{
						ret = 0;
						for( k = 0;k<sendNum;k++)
						{
							zb_send_cmd(curinverter,(char *)buff,packlen/2);
							ret = zb_get_reply(data,curinverter);
							if(ret > 0)
							{
								ResponseZigbeeTransimission(1,curinverter->uid,ret*2,(unsigned char *)data);
								break;
							}
						}
						if(ret<=0)
						{
							ResponseZigbeeTransimission(0,curinverter->uid,ret*2,(unsigned char *)data);
						}
					}
				}
			}	
					
			break;
		case 2:	//任意命令
			for(i = 0;i<uidNum;i++)
			{
				memcpy(UID,&Uidlist[0+12*i],12);
				curinverter = inverterInfo;
				for(j=0; j<MAXINVERTERCOUNT; j++, curinverter++)
				{
					if(!memcmp(curinverter->uid,UID,12))
					{
						ret = 0;
						for( k = 0;k<sendNum;k++)
						{
							zb_transmission( (char *)buff, packlen/2);
							ret = zb_transmission_reply(data);
							if(ret > 0)
							{
								ResponseZigbeeTransimission(1,curinverter->uid,ret*2,(unsigned char *)data);
								break;
							}
						}
						if(ret<=0)
						{
							ResponseZigbeeTransimission(0,curinverter->uid,ret*2,(unsigned char *)data);
						}
						
					}
				}
			}			
			break;
		default:
					
			break;
	}
}


void process_ZigBeeTransimission(void)
{
	FILE *fp;
	unsigned short packLen = 0,uidNum = 0,sendNum = 0,TimeInterval = 0;
	char temp[6] = {'\0'};
	int flag;
	char *buff = malloc(2048);
	memset(buff,0x00,2048);
	
	fp = fopen("/tmp/tran.inf", "r");
	if (fp) {
		fgets(buff, 2048, fp);
		fclose(fp);
		
		memcpy(temp,&buff[48],2);
		temp[2] = '\0';
		sendNum = atoi(temp);
		memcpy(temp,&buff[50],2);
		temp[2] = '\0';
		TimeInterval = atoi(temp);
		memcpy(temp,&buff[52],3);
		temp[3] = '\0';
		packLen = atoi(temp);
		memcpy(temp,&buff[55+packLen],3);
		temp[3] = '\0';
		uidNum = atoi(temp);
		sscanf(&buff[30], "%1d", &flag);	//标志位 0:广播0x55    1:单播0x55    2:任意命令
		
		printf("sendNum:%d TimeInterval:%d packLen:%d uidNum:%d flag:%d\n",sendNum,TimeInterval,packLen,uidNum,flag);
		ZigBeeTransimission(flag,sendNum,TimeInterval,packLen,&buff[55],uidNum,&buff[58+packLen]);
		unlink("/tmp/tran.inf");
		
	}
	
	free(buff);
	buff = NULL;
}

//A173命令
int transmission_ZigBeeInfo(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	
	strncpy(timestamp, &recvbuffer[31], 14);

	//格式检查
	if(formatTransmission(recvbuffer) == 1)
	{
		ack_flag = FORMAT_ERROR;
	}else
	{
		echo("/tmp/tran.inf",recvbuffer);
	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A173", timestamp, ack_flag);
	return 0;	
}
