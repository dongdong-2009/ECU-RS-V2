#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include "debug.h"
#include "zigbee.h"
#include "remote_update.h"
#include "crc.h"
#include <dfs_posix.h>
#include "serverfile.h"
#include "ECUCollect.h"
#include "rtc.h"

extern struct rt_device serial4;		//����4ΪZigbee�շ�����
#define ZIGBEE_SERIAL (serial4)
extern unsigned char ECUCommThreadFlag;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


typedef enum  remotetype{
  Remote_UpdateSuccessful    	= 0,
  Remote_UpdateFailed_SendStart	= 1,
  Remote_UpdateFailed_FillingPackageStartNoResponse   	= 2,
  Remote_UpdateFailed_FillingPackageNoResponse   	= 3,
  Remote_UpdateFailed_Other	= 4,
  Remote_UpdateFailed_ModelUnMatch     	= 5,
  Remote_UpdateFailed_OpenFile    	= 6,
  Remote_UpdateFailed_CRC	= 7,
  Remote_UpdateFailed_CRCNoResponse     	= 8,
}eRemoteType;

//���������صĴ���ת��Ϊ���͸�EMA�Ĵ����
eRemoteType getResult(int ret)
{
	if(0 == ret)
	{
		return Remote_UpdateSuccessful;
	}else if(1 == ret)
	{
		return Remote_UpdateFailed_SendStart;
	}else if(2 == ret)
	{
		return Remote_UpdateFailed_ModelUnMatch;
	}else if(3 == ret)
	{
		return Remote_UpdateFailed_OpenFile;
	}else if(4 == ret)
	{
		return Remote_UpdateFailed_FillingPackageStartNoResponse;
	}else if(5 == ret)
	{
		return Remote_UpdateFailed_FillingPackageNoResponse;
	}else if(6 == ret)
	{
		return Remote_UpdateFailed_CRC;
	}else if(7 == ret)
	{	
		return Remote_UpdateFailed_CRCNoResponse;
	}else
	{
		return Remote_UpdateFailed_Other;
	}
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                        */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -3 : ECU��֧�ָû�����������                                            */
/*   -2 : û��BIN�ļ�                                                        */
/*   -1 : ����ʧ��                                                           */
/*    1 ���·��ɹ�                                                           */
/*****************************************************************************/
int Sendupdatepackage_start(inverter_info *inverter)	//���͵��㿪ʼ���ݰ�
{
	int i=0;
	int crc;
	unsigned char data[256];
	unsigned char sendbuff[76]={0x00};
	int pos = 0;
	char flag;	
	int fd = -1;
	FILE *fp = NULL;
	
	if(0x01 ==  inverter->model)
	{
		fd=open("/FTP/UPOPT700.BIN",O_RDONLY, 0);
	}else
	{
		return -3;		//ECU��֧�ָû�����������
	}
	
	if(fd < 0)
	{
		printmsg(ECU_DBG_COMM,"No BIN");
		return -2;	//û�ж�Ӧ��������
	}
	pos = lseek(fd,0,SEEK_END);
	close(fd);
	pos=(pos-0.5)/64+1;
	printdecmsg(ECU_DBG_COMM,"pos",pos);

	sendbuff[8]=pos/256;
	sendbuff[9]=pos%256;
	sendbuff[10]=0x01;
	sendbuff[11]=0x01;
	fp=fopen("/config/upload1.con","r");
	if(fp==NULL)
	{}
	else
	{
		flag=fgetc(fp);
		fclose(fp);
		sendbuff[10]=flag-0x30;
	}

	fp=fopen("/config/upload2.con","r");
	if(fp==NULL)
	{}
	else
	{
		flag=fgetc(fp);
		fclose(fp);
		sendbuff[11]=flag-0x30;
	}
	
	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x01;
	sendbuff[5]=0x80;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

	crc=crc_array(&sendbuff[2],70);
	sendbuff[72]=crc/256;
	sendbuff[73]=crc%256;
	for(i=0;i<7;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,76);
		printhexmsg(ECU_DBG_COMM,"Ysend",(char *)sendbuff,76);
		printmsg(ECU_DBG_COMM,"Sendupdatepackage_start");
		zb_get_reply((char *)data,inverter);
		if((0x03 == data[2]) && (0x02 == data[3]) && (0x18 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[8]) && (0xFE == data[9]))
		{
			crc=crc_array(&data[2],4);
			if((data[6]==crc/256)&&(data[7]==crc%256))
			break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}

	if(i>=7)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�-1
		return -1;
	else
		return 1;							//������������

}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -2 : û��BIN�ļ�                                                        */
/*   -1 : ECU��֧�ָû�����������                                            */
/*    1 ���·��������ɹ�                                                           */
/*****************************************************************************/
int Sendupdatepackage_single(inverter_info *inverter)	//���͵������ݰ�
{
	int fd;
	int crc;
	int i=0,package_num=0;
	unsigned char package_buff[100];
	unsigned char sendbuff[76]={0xff};
	printf("**********\n2\n*************");
	
	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x02;
	sendbuff[5]=0x40;
	sendbuff[72]=0x00;
	sendbuff[73]=0x00;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

	if(0x01 ==  inverter->model)
	{
		fd=open("/FTP/UPOPT700.BIN", O_RDONLY,0);
	}else
	{
		return -1;		//�û��Ͳ�֧������
	}
	
	if(fd>=0)
	{
		while(read(fd,package_buff,64)>0){
			sendbuff[6]=package_num/256;
			sendbuff[7]=package_num%256;
			for(i=0;i<64;i++){
				sendbuff[i+8]=package_buff[i];
			}
			crc=crc_array(&sendbuff[2],70);
			sendbuff[72]=crc/256;
			sendbuff[73]=crc%256;

			zb_send_cmd(inverter,(char *)sendbuff,76);
			package_num++;
			printdecmsg(ECU_DBG_COMM,"package_num",package_num);
			printhexmsg(ECU_DBG_COMM,"send",(char *)sendbuff,76);
			rt_thread_delay(15);
			memset(package_buff, 0, sizeof(package_buff));
		}
		close(fd);
		return 1;
	}
	else
		return -2;		//û��BIN�ļ�
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                       */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -2 :������������                                            */
/*   -1 :������7��û��Ӧ�����                                         */
/*   0 : ������ʼָ���޻�Ӧ                                                        */
/*    1 ���ɹ�����                                                   */
/*****************************************************************************/
int Complementupdatepackage_single(inverter_info *inverter)	//���©�������ݰ�������
{
	int crc;
	int fd;
	int ret;
	int i=0,k=0;
	unsigned char data[256];
	unsigned char checkbuff[76]={0x00};
	unsigned char sendbuff[76]={0xff};
	unsigned char package_buff[100];

	checkbuff[0]=0xfc;
	checkbuff[1]=0xfc;
	checkbuff[2]=0x02;
	checkbuff[3]=0x03;
	checkbuff[4]=0x04;
	checkbuff[5]=0x20;
	checkbuff[74]=0xfe;
	checkbuff[75]=0xfe;

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x02;
	sendbuff[3]=0x03;
	sendbuff[4]=0x02;
	sendbuff[5]=0x4f;
	sendbuff[72]=0x00;
	sendbuff[73]=0x00;
	sendbuff[74]=0xfe;
	sendbuff[75]=0xfe;

	clear_zbmodem();
	do{
		zb_send_cmd(inverter,(char *)checkbuff,76);
		printmsg(ECU_DBG_COMM,"Complementupdatepackage_single_checkbuff");
		ret = zb_get_reply((char *)data,inverter);
		rt_thread_delay(RT_TICK_PER_SECOND);
		printdecmsg(ECU_DBG_COMM,"ret",ret);
		i++;
	}while((-1==ret)&&(14!=ret)&&(i<7));

	//��ȡ������ʼָ��ɹ�
	if((0x03 == data[2]) && (0x02 == data[3]) && (14 == ret) && (0x42 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[12]) && (0xFE == data[13]))
	{
		fd=open("/FTP/UPOPT700.BIN", O_RDONLY,0);

		if(fd>=0){
			while((data[8]*256+data[9])>0)
			{
				lseek(fd,(data[6]*256+data[7])*64,SEEK_SET);
				memset(package_buff, 0, sizeof(package_buff));
				read(fd, package_buff, 64);
				sendbuff[6]=data[6];
				sendbuff[7]=data[7];
				for(k=0;k<64;k++){
					sendbuff[k+8]=package_buff[k];
				}
				crc=crc_array(&sendbuff[2],70);
				sendbuff[72]=crc/256;
				sendbuff[73]=crc%256;

				for(i=0;i<7;i++)
				{
					zb_send_cmd(inverter,(char *)sendbuff,76);
					ret = zb_get_reply((char *)data,inverter);
					if((0x03 == data[2]) && (0x02 == data[3]) && (0x24 == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[12]) && (0xFE == data[13]))
					{
						crc=crc_array(&data[2],8);
						if((data[10]==crc/256)&&(data[11]==crc%256))
							break;
					}
				}
				if(i>=7)
				{
					printmsg(ECU_DBG_COMM,"Complementupdatepackage single 3 times failed");
					return -1;		//������3��û��Ӧ�����
				}
				printdecmsg(ECU_DBG_COMM,"Complement_package",(data[8]*256+data[9]));
				rt_thread_delay(3);
			}
			close(fd);
			return 1;	//�ɹ�����
		}else
		{
			return -2;	//������BIN�ļ�
		}
		
	}
	else
	{
		//������ʼָ�� �޻�Ӧ
		printmsg(ECU_DBG_COMM,"Complement checkbuff no response");
		return 0;	
	}
}

int Update_start(inverter_info *inverter)		//���͸���ָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={'\0'};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x05;
	sendbuff[3]=0xa0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_COMM,"Update_start");
		ret = zb_get_reply_update_start((char *)data,inverter);
		if((8 == ret) && (0x05 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//���³ɹ�
			return 1;
		if((8 == ret) && (0xe5 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//����ʧ�ܣ���ԭ
			return -1;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0
		return 0;

	return 0;
}

int Update_success_end(inverter_info *inverter)		//���³ɹ�����ָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={'\0'};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x03;
	sendbuff[3]=0xc0;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_COMM,"Update_success_end");
		ret = zb_get_reply((char *)data,inverter);
		if((0 == ret%8) && (0x3C == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))
			break;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�-1
		return -1;
	else
	{
		printmsg(ECU_DBG_COMM,"Update_success_end successful");
		return 1;
	}

}

int Restore(inverter_info *inverter)		//���ͻ�ԭָ��
{
	int ret;
	int i=0;
	unsigned char data[256]={'\0'};
	unsigned char sendbuff[74]={0x00};

	sendbuff[0]=0xfc;
	sendbuff[1]=0xfc;
	sendbuff[2]=0x06;
	sendbuff[3]=0x60;
	sendbuff[72]=0xfe;
	sendbuff[73]=0xfe;

	for(i=0;i<3;i++)
	{
		zb_send_cmd(inverter,(char *)sendbuff,74);
		printmsg(ECU_DBG_COMM,"Restore");
		ret = zb_get_reply_restore((char *)data,inverter);
		if((8 == ret) && (0x06 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭ�ɹ�
			return 1;
		if((8 == ret) && (0xe6 == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[6]) && (0xFE == data[7]))//��ԭʧ��
			return -1;
	}

	if(i>=3)								//�������3��ָ����Ȼû�з�����ȷָ��򷵻�0
		return 0;
	
	return 0;
}
void crc_add_crc(unsigned char *pchMsg, int wDataLen,int cc)
{
	unsigned short wCRCTalbeAbs[16] = {0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
	unsigned short wCRC = cc;
	int i;
	unsigned char chChar;
	for (i = 0; i < wDataLen; i++)
	{
		chChar = *pchMsg++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	*pchMsg++ = wCRC & 0xff;
	*pchMsg =  (wCRC>>8)&0xff;

}


/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*                                        */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   inverter : inverter struct                                              */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   -3 : ECU��֧�ָû�����������                                            */
/*   -2 : û����Ӧ CRCУ����ָ��                                                       */
/*   -1 : У��ʧ��                                                           */
/*     1 : У��ɹ�                                                           */
/*****************************************************************************/

int crc_bin_file(inverter_info *inverter)
{
	unsigned char checkbuff[76]={'\0'};
	unsigned char data[100]={'\0'};
	int crc,i = 0;
	int ret;
	if(0x01 ==  inverter->model)
	{
		crc=crc_file("/FTP/UPOPT700.BIN");
	}else
	{
		return -3;		//�û��Ͳ�֧������
	}
	
 	checkbuff[0]=0xFC;
	checkbuff[1]=0xFC;
	checkbuff[2]=0x02;
	checkbuff[3]=0x03;
	checkbuff[4]=0x07;
	checkbuff[5]=0xE0;
	checkbuff[8]=crc/256;
	checkbuff[9]=crc%256;
	checkbuff[74]=0xFE;
	checkbuff[75]=0xFE;
	crc=crc_array(&checkbuff[2],70);
	checkbuff[72]=crc/256;
	checkbuff[73]=crc%256;

	for(i=0;i<7;i++)
	{
		zb_send_cmd(inverter,(char *)checkbuff,76);
		printhexmsg(ECU_DBG_COMM,"Ysend",(char *)checkbuff,76);
		ret = zb_get_reply((char *)data,inverter);
		if((data[2]==0x03)&&(data[3]==0x02)&&(ret%10==0)&&(data[0]==0xFB)&&(data[1]==0xFB)&&(data[4]==0x01)&&(data[8]==0xFE)&&(data[9]==0xFE)) break;
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	
	if((data[2]==0x03)&&(data[3]==0x02)&&(ret%10==0)&&(data[0]==0xFB)&&(data[1]==0xFB)&&(data[4]==0x01)&&(data[8]==0xFE)&&(data[9]==0xFE))
	{
		crc=crc_array(&data[2],4);
		if((data[6]==crc/256)&&(data[7]==crc%256))
		{
			if(data[5]==0x7A)		//CRCУ��ɹ�
				return 1;
			else if(data[5]==0x7E)	//CRCУ��ʧ��
				return -1;
			else					
				return -1;
		}else
		{
			return -1;
		}
	}else
	{		//û����Ӧ CRCУ����ָ�� 
			return -2;
	}

}


/* ������̨�����
 *
 * ����ֵ    0 �����ɹ�				
 * 			1 ���Ϳ�ʼ�������ݰ�ʧ��
 * 		 	2 û�ж�Ӧ�ͺŵ������(������δ���������������Ļ����벻֧������)
 * 		 	3 �������ļ�ʧ��(�ļ�������)
 * 		 	4 ������ʼָ��ʧ��
 * 		 	5 ��������Ӧ
 * 		 	6 ����ʧ��CRCУ��ʧ��
 * 		 	7 ��������ӦCRCУ������Ӧ
 */
int remote_update_single(inverter_info *inverter)
{
	int ret_sendsingle,ret_complement,ret_update_start=0,ret_start = 0;

	ret_start = Sendupdatepackage_start(inverter);
	if(1==ret_start)						//1Զ�̸�������ָ�������������
	{
		printmsg(ECU_DBG_COMM,"Sendupdatepackage_start_OK");
		ret_sendsingle = Sendupdatepackage_single(inverter);		//2���ݰ�����

		if(-1==ret_sendsingle)		//ECU��֧�ָû�����������
		{
			printmsg(ECU_DBG_COMM,"MODEL NO Match");
			return 2;
		}
		else if(-2==ret_sendsingle)		//�ļ�������ȫ���
		{
			return 3;	// �������ļ�ʧ��
		}
		else //if(1==ret_sendsingle)		//�ļ�������ȫ�����
		{
			printmsg(ECU_DBG_COMM,"Complementupdatepackage_single");
			ret_complement = Complementupdatepackage_single(inverter);	//3����ָ��

			if(-1==ret_complement)		//����������3��û��Ӧ�Ͳ�������ֱ�ӳ���512�������
			{
				return 5;		//��������Ӧ
			}
			else if(1==ret_complement)		//�ɹ�������ȫ�����
			{
				ret_update_start=crc_bin_file(inverter);

				if(1==ret_update_start)		//���³ɹ�
				{	
					printmsg(ECU_DBG_COMM,"Update start successful");
					return 0;
				}
				else if(-1==ret_update_start)	//����У��ʧ��
				{
					printmsg(ECU_DBG_COMM,"Update_start_failed");
					return 6;
				}
				else if(-2==ret_update_start)	//û����Ӧ CRCУ����ָ��
				{
					printmsg(ECU_DBG_COMM,"Update start no response");
					return 7;
					
				}else 		//ECU��֧�ָû�����������
				{
					return 2;
				}
			}
			else if(0==ret_complement)	//������ʼָ������Ӧ
			{
				return 4;
	
			}else //if (-2==ret_complement)
			{
				return 3;
			}
		}
	}
	else if (-3==ret_start)	//ECU��֧�ָû�����������
	{
		return 2;
	}
	else if (-2==ret_start)	//������BIN�ļ�
	{
		return 3;	// �������ļ�ʧ��
	}
	else			//����-1 
		return 1;		//���Ϳ�ʼ�������ݰ�ʧ��
	
}

int remote_update(inverter_info *firstinverter)
{
	int i=0, j=0,version_ret = 0;
	int update_result = 0;
	char data[200];
	char splitdata[4][32];
	char pre_Time[15] = {"/0"};
	char Time[15] = {"/0"};
	char inverter_result[128];
	inverter_info *curinverter = firstinverter;
	eRemoteType remoteTypeRet = Remote_UpdateSuccessful; 
	ECUCommThreadFlag = EN_ECUHEART_DISABLE;
			
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->uid)); i++,curinverter++)
	{
		//��ȡ����ID��
		if(1 == read_line("/home/data/upinv",data,curinverter->uid,12))
		{
			splitString(data,splitdata);
			memset(data,0x00,200);
			
			if(1 == atoi(splitdata[3]))
			{
				printmsg(ECU_DBG_COMM,curinverter->uid);
				apstime(pre_Time);
				update_result = remote_update_single(curinverter);
				printdecmsg(ECU_DBG_COMM,"Update",update_result);
				apstime(Time);
				rt_thread_delay(RT_TICK_PER_SECOND * 10);
				if(0 == update_result)
				{	//ֻ�������ɹ��˲Ų�ѯ�汾��
					for(j=0;j<3;j++)
					{
						if(1 == zb_query_heart_data(curinverter))
						{
							version_ret = 1;
							break;
						}
					}

					if(1 == version_ret)	//��ȡ���汾��
					{
						sprintf(data,"%s,%d,%s,0\n",curinverter->uid,curinverter->version,Time);
					}else		//δ��ȡ���汾��
					{
						sprintf(data,"%s,%d,%s,0\n",curinverter->uid,0,Time);
					}
				}else
				{
					sprintf(data,"%s,%d,%s,0\n",curinverter->uid,curinverter->version,Time);
				}
				
				remoteTypeRet = getResult(update_result);
				sprintf(inverter_result, "%s%02d%06d%sEND", curinverter->uid, remoteTypeRet,curinverter->version, Time);
				save_inverter_parameters_result2(curinverter->uid, 147,inverter_result);

								
#if 0
				memset(inverter_result,0x00,128);
				sprintf(inverter_result, "%s,%02d,%06d,%s,%s\n", curinverter->uid, remoteTypeRet,curinverter->version, pre_Time,Time);
				for(j=0;j<3;j++)
				{		
					if(1 == insert_line("/tmp/update.tst",inverter_result))
						break;
					rt_thread_delay(RT_TICK_PER_SECOND);
				}	
				memset(inverter_result,0x00,128);
				rt_thread_delay(RT_TICK_PER_SECOND*10);
#else
				//ɾ��ID������
				delete_line("/home/data/upinv","/home/data/upinv.t",curinverter->uid,12);
				
				for(j=0;j<3;j++)
				{		
					if(1 == insert_line("/home/data/upinv",data))
						break;
					rt_thread_delay(RT_TICK_PER_SECOND);
				}
#endif
			}
		}
		
	}
	ECUCommThreadFlag = EN_ECUHEART_ENABLE;
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void remote(void)
{
	remote_update(inverterInfo);
}
FINSH_FUNCTION_EXPORT(remote, eg:remote());
#endif
