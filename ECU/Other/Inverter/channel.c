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
/* �ŵ����� */
int process_channel()
{
	int oldChannel, newChannel;
	
	if (channel_need_change()) {
		//��ȡ���ǰ����ŵ�
		oldChannel = getOldChannel();
		newChannel = getNewChannel();
		printf("%02x %02x\n",oldChannel,newChannel);
		//�޸��ŵ�
		changeChannelOfInverters(oldChannel, newChannel);

		//��ECU�����ŵ����������ļ�
		saveECUChannel(newChannel);
	
		//��ձ�־λ
		unlink("/tmp/changech.con");
		unlink("/tmp/old_chan.con");
		unlink("/tmp/new_chan.con");
	}
	return 0;
}

/* �ж��Ƿ���Ҫ�ı��ŵ� */
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


// ��ȡ�ŵ�����Χ��11~26��16���ŵ�
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



// ��ȡ�ŵ�����Χ��11~26��16���ŵ�
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
	return 0; //δ֪�ŵ�
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
	return 16; //Ĭ���ŵ�
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
	//��ȡ�����ID
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//��Ч�������ѵ
	{
		curinverter->status.flag = 1;
			num++;
	}

	//�����ŵ�
	if (num > 0) {
		//ԭ�ŵ���֪
		if (oldChannel) {
			//����ECU�ŵ�Ϊԭ�ŵ�
			zb_restore_ecu_panid_0xffff(oldChannel);

			//����ÿ̨������ŵ�
			curinverter = inverterInfo;
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//��Ч�������ѵ
			{
				if(curinverter->status.flag == 1)
				{
					zb_change_inverter_channel_one(curinverter->uid, newChannel);
				}	
		
			}
		}
		//ԭ�ŵ�δ֪
		else {
			//ECU��ÿһ���ŵ�����ÿһ̨��������͸����ŵ���ָ��
			for (nChannel=11; nChannel<=26; nChannel++ ) {
				zb_restore_ecu_panid_0xffff(nChannel);
				curinverter = inverterInfo;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++, curinverter++)			//��Ч�������ѵ
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
