#include "ECUCollect.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rtc.h"


extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];


void Collect_Client_Record(void)
{
	char *client_Data = NULL;
	int length = 0; //��ǰ��������λ��
	int i = 0;				//��ѵ����
	inverter_info *curinverter = inverterInfo;
	int commNum = 0; 	//ͨѶ�ϵ����������
	
	if(ecu.validNum > 0)
	{
		client_Data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT);
		memcpy(client_Data,"APS16AAAAA0002AAA1",18);
		memcpy(&client_Data[18],ecu.ECUID12,12);
		memset(&client_Data[30],'0',63);
		memcpy(&client_Data[93],"END",3);
		length = 96;
		//�ɼ�һ������
		commNum = 0;
		curinverter = inverterInfo;
		for(i = 0;i< ecu.validNum; i++)
		{
			//�ɼ�ÿһ���Ż���������
			//�ж������Ƿ����һ���и��£���������ˣ�����Ҫ�ϴ������û���¾Ͳ��ϴ�
			if(Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime) > 0)
			{
				commNum++;		
				//��ȡ��ǰʱ��
				apstime(curinverter->CurCollectTime);
				//�ڲ������и���
				//ID	12�ֽ�
				client_Data[length++] = (curinverter->uid[0]/16) + '0';
				client_Data[length++] = (curinverter->uid[0]%16) + '0';
				client_Data[length++] = (curinverter->uid[1]/16) + '0';
				client_Data[length++] = (curinverter->uid[1]%16) + '0';
				client_Data[length++] = (curinverter->uid[2]/16) + '0';
				client_Data[length++] = (curinverter->uid[2]%16) + '0';
				client_Data[length++] = (curinverter->uid[3]/16) + '0';
				client_Data[length++] = (curinverter->uid[3]%16) + '0';
				client_Data[length++] = (curinverter->uid[4]/16) + '0';
				client_Data[length++] = (curinverter->uid[4]%16) + '0';
				client_Data[length++] = (curinverter->uid[5]/16) + '0';
				client_Data[length++] = (curinverter->uid[5]%16) + '0';
				//�����ѹ 6�ֽ�
				sprintf(&client_Data[length],"%06d",curinverter->PV_Output);
				length += 6;
				//������� 6�ֽ�
				memcpy(&client_Data[length],"000000",6);
				length += 6;
				//������� 6�ֽ�
				memcpy(&client_Data[length],"000000",6);
				length += 6;
				//������� 10�ֽ�
				memcpy(&client_Data[length],"0000000000",10);
				length += 10;
				//�¶� 3�ֽ�
				memcpy(&client_Data[length],"000",3);
				length += 3;
				//Optimizer_pv1  1�ֽ�
				client_Data[length++]  = 1;
				// PV1�����ѹ 6�ֽ�
				sprintf(&client_Data[length],"%06d",curinverter->PV1);
				length += 6;
				// pv1������� 6�ֽ�
				sprintf(&client_Data[length],"%06d",curinverter->PI);
				length += 6;
				//�����ǰһ�ֵ���С����һ�ֵĵ���  ����Ĭ��Ϊ�������ˣ�����ֱ�ӻ�ȡ������Ϊ��ʱ��˲ʱ����
				if(curinverter->Last_PV1_Energy > curinverter->PV1_Energy)
				{
					sprintf(&client_Data[length],"%06d",(curinverter->Power1*100));
					length += 6;
					sprintf(&client_Data[length],"%010d",(curinverter->PV1_Energy*1000000));
					length += 10;
				}else
				{
					unsigned short Power1;
					//����=(��ǰһ�ֵ���-��һ�ֵ���)/ʱ��Ĳ�ֵ
					Power1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy)/Time_difference(curinverter->CurCollectTime,curinverter->LastCollectTime);
					//pv1���빦��(����õ�ƽ������)
					
					//pv1�������(���ּ����ֵ)
				}
										
				//Optimizer_pv2  1�ֽ�
				client_Data[length++]  = 2;
				// PV2�����ѹ 6�ֽ�
				sprintf(&client_Data[length],"%06d",curinverter->PV2);
				length += 6;
				// pv2������� 6�ֽ�
				sprintf(&client_Data[length],"%06d",curinverter->PI);
				length += 6;
				//�����ǰһ�ֵ���С����һ�ֵĵ���  ����Ĭ��Ϊ�������ˣ�����ֱ�ӻ�ȡ������Ϊ��ʱ��˲ʱ����
				if(curinverter->Last_PV2_Energy > curinverter->PV2_Energy)
				{
					sprintf(&client_Data[length],"%06d",(curinverter->Power2*100));
					length += 6;
					sprintf(&client_Data[length],"%010d",(curinverter->PV2_Energy*1000000));
					length += 10;
				}else
				{
					unsigned short Power2;
					//����=(��ǰһ�ֵ���-��һ�ֵ���)/ʱ��Ĳ�ֵ
					Power2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy)/Time_difference(curinverter->CurCollectTime,curinverter->LastCollectTime);
					//pv2���빦��(����õ�ƽ������)
				
					//pv2�������(���ּ����ֵ)
				}
					
				//END
				client_Data[length++] = 'E';
				client_Data[length++] = 'N';
				client_Data[length++] = 'D';
				
				
						
				memcpy(curinverter->LastCollectTime,curinverter->CurCollectTime,15);
				curinverter->Last_PV1_Energy = curinverter->PV1_Energy;
				curinverter->Last_PV2_Energy = curinverter->PV2_Energy;
				//�������ݵ��ļ���
				
						
			}
					
					
			curinverter++;
		}
		client_Data[length++] = '\n';
		//����ECU�����������
		
		//��������
		printf("client Data:%s\n",client_Data);
		free(client_Data);
		client_Data = NULL;
	}
}



//���߳���Ҫ����������ݵĲɼ�����
void ECUCollect_thread_entry(void* parameter)
{

	int CollectClientThistime=0, CollectClientDurabletime=65535, CollectClientReportinterval=300;			//�ɼ��������ʱ�����
	int CollectControlThistime=0, CollectControlDurabletime=65535, CollectControlReportinterval=900;	//�ɼ�Զ�̿�������ʱ�����
	
	while(1)
	{
		if(compareTime(CollectClientDurabletime ,CollectClientThistime,CollectClientReportinterval))
		{
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     DATA  Start-------------------------");
			//5���Ӳɼ���صķ���������
			CollectClientThistime = acquire_time();
		
			
			
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     DATA  End-------------------------");

		}
		
		
		
		if(compareTime(CollectControlDurabletime ,CollectControlThistime,CollectControlReportinterval))
		{	
			//�ɼ��������Զ�̿�������
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     Control DATA  Start-------------------------");

			CollectControlThistime = acquire_time();
			
			printmsg(ECU_DBG_COLLECT,"ECUCollect_thread_entry     Control DATA  End-------------------------");

		}

		rt_thread_delay(RT_TICK_PER_SECOND);
		CollectClientDurabletime = acquire_time();		
		CollectControlDurabletime = acquire_time();			
	}
}
