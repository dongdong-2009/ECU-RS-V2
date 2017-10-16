/*****************************************************************************/
/*  File      : channel.c                                                    */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include <dfs_posix.h> 
#include "channel.h"
#include "zigbee.h"
#include "serverfile.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 信道操作 */
int process_channel()
{
	int oldChannel, newChannel;
	
	if (channel_need_change()) {
		//获取变更前后的信道
		oldChannel = getOldChannel();
		newChannel = getNewChannel();
		printf("%02x %02x\n",oldChannel,newChannel);
		//修改信道
		changeChannelOfInverters(oldChannel, newChannel);

		//将ECU的新信道存入配置文件
		saveECUChannel(newChannel);
	
		//清空标志位
		unlink("/tmp/changech.con");
		unlink("/tmp/old_chan.con");
		unlink("/tmp/new_chan.con");
	}
	return 0;
}

/* 判断是否需要改变信道 */
int channel_need_change()
{
	FILE *fp;
	char buff[2] = {'\0'};

	fp = fopen("/tmp/changech.con", "r");
	if (fp) {
		fgets(buff, 2, fp);
		fclose(fp);
	}

	return ('1' == buff[0]);
}

int saveChannel_change_flag()
{
	echo("/tmp/changech.con","1");
	echo("/config/limiteid.con","1");
	return 0;
}


// 获取信道，范围：11~26共16个信道
int saveOldChannel(unsigned char oldChannel)
{
	FILE *fp;
	char buffer[3] = {'\0'};
	
	fp = fopen("/tmp/old_chan.con", "w");
	if (fp) {
		sprintf(buffer,"%d",oldChannel);
		fputs(buffer, fp);
		fclose(fp);
	}
	return 0;
}
int saveNewChannel(unsigned char newChannel)
{
	FILE *fp;
	char buffer[3] = {'\0'};

	fp = fopen("/tmp/new_chan.con", "w");
	if (fp) {
		sprintf(buffer,"%d",newChannel);
		fputs(buffer, fp);
		fclose(fp);
	}
	return 0;
}



// 获取信道，范围：11~26共16个信道
int getOldChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/old_chan.con", "r");
	if (fp) {
		printf("getOldChannel:%s\n",buffer);
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 0; //未知信道
}
int getNewChannel()
{
	FILE *fp;
	char buffer[4] = {'\0'};

	fp = fopen("/tmp/new_chan.con", "r");
	if (fp) {
		printf("getNewChannel:%s\n",buffer);
		fgets(buffer, 4, fp);
		fclose(fp);
		return atoi(buffer);
	}
	return 16; //默认信道
}

int saveECUChannel(int channel)
{
	FILE *fp;
	char buffer[5] = {'\0'};

	snprintf(buffer, sizeof(buffer), "0x%02X", channel);
	fp = fopen("/config/channel.con", "w");
	if (fp) {
		fputs(buffer, fp);
		fclose(fp);
		ecu.channel = channel;
		return 1;
	}
	return 0;
}


void changeChannelOfInverters(int oldChannel, int newChannel)
{
	int num = 0,i = 0,nChannel;
	inverter_info *curinverter = inverterInfo;
	//获取逆变器ID
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//有效逆变器轮训
	{
		curinverter->status.flag = 1;
			num++;
	}

	//更改信道
	if (num > 0) {
		//原信道已知
		if (oldChannel) {
			//更改ECU信道为原信道
			zb_restore_ecu_panid_0xffff(oldChannel);

			//更改每台逆变器信道
			curinverter = inverterInfo;
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//有效逆变器轮训
			{
				if(curinverter->status.flag == 1)
				{
					zb_change_inverter_channel_one(curinverter->uid, newChannel);
				}	
		
			}
		}
		//原信道未知
		else {
			//ECU在每一个信道都给每一台逆变器发送更改信道的指令
			for (nChannel=11; nChannel<=26; nChannel++ ) {
				zb_restore_ecu_panid_0xffff(nChannel);
				curinverter = inverterInfo;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//有效逆变器轮训
				{
					if(curinverter->status.flag == 1)
					{
						zb_change_inverter_channel_one(curinverter->uid, newChannel);
					}	
				}
			}
		}
	}

}
