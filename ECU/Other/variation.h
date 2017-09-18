/*****************************************************************************/
/* File      : variation.h                                                        */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __VARIATION_H__
#define __VARIATION_H__

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define MAXINVERTERCOUNT 								100	//�����������
#define INVERTERLENGTH 									22	//�����������  //���ֻ�ͨѶ
//Client ���ͨ�Ų���
#define CLIENT_RECORD_HEAD							20
#define CLIENT_RECORD_ECU_HEAD					78
#define CLIENT_RECORD_INVERTER_LENGTH		104
#define CLIENT_RECORD_OTHER							100

#define CONTROL_RECORD_HEAD							18
#define CONTROL_RECORD_ECU_HEAD					33
#define CONTROL_RECORD_INVERTER_LENGTH	41
#define CONTROL_RECORD_OTHER						100


#define CONTROL_RECORD_ALARM_ECU_HEAD						(14*99+36)




#pragma pack(push)  
#pragma pack(1) 

typedef struct
{
    unsigned short bind_status:1;		// ��״̬  
    unsigned short mos_status:1;			//���ػ�״̬ :  1 ��    0 ��
	unsigned short function_status:1;	//���ܿ���״̬: 1 ��    0 ��
	unsigned short heart_Failed_times:3; // ����ͨ��ʧ�ܴ���  ��������3��ʱ��Ĭ�ϸ�RSD2Ϊ�ػ�״̬
	unsigned short pv1_low_voltage_pritection:1;	// PV1Ƿѹ����
	unsigned short pv2_low_voltage_pritection:1;	// PV2Ƿѹ����
	unsigned short device_Type:4;					//�豸����  0:�����豸 1������豸
	unsigned short channel_failed:1;				//�޸��ŵ�ʧ�ܱ�־
	unsigned short comm_status:1;					//�ɼ�����ͨѶ״̬
	unsigned short unused:3;						//δʹ�ñ���  ����
}status_t;


typedef struct inverter_info_t{
	unsigned char uid[6];		//�����ID�������ID��BCD���룩
	unsigned short heart_rate;	//��������
	unsigned short off_times;	//������ʱ����
	status_t status;			//����״̬��Ϣ 
	unsigned char channel;		//�ŵ�״̬
	unsigned char find_channel; //���ҵ��ŵ�
	unsigned char restartNum;	//һ���ڵ���������
	unsigned short PV1;		//PV1�����ѹ  ���� 0.1V
	unsigned short PV2;		//PV2�����ѹ  ���� 0.1V
	unsigned char PI;		//������� 	����0.1A
	unsigned short PV_Output; //�����ѹ ����0.1V
	unsigned short Power1;	//PV1���빦��  ����1W
	unsigned short Power2;	//PV2���빦��  ����1W 
	unsigned char RSSI;	//�ź�ǿ��
	unsigned int PV1_Energy;//��ǰһ��PV1������	���� 1����
	unsigned int PV2_Energy;//��ǰһ��PV2������	���� 1����
	unsigned char Mos_CloseNum;//�豸�ϵ��MOS�ܹضϴ���
	char LastCommTime[15];	//RSD���һ��ͨѶ�ϵ�ʱ��		
	//��һ����ص����ݣ��������һ��ָ����5����һ��
	char LastCollectTime[15];	//��һ�ֲɼ�ʱ�����һ��ͨѶʱ��
	unsigned int Last_PV1_Energy;//��һ��PV1������ ָ����5����ǰ��һ��
	unsigned int Last_PV2_Energy;//��һ��PV2������ ָ����5����ǰ��һ��
	double AveragePower1; //5����ƽ������1
	double AveragePower2; //5����ƽ������2
	unsigned int EnergyPV1;		//��ǰһ�ֵ���	���� 1����
	unsigned int EnergyPV2;		//��ǰһ�ֵ���	���� 1����
	
}inverter_info;

typedef struct ecu_info_t{
	char ECUID12[13];
	char ECUID6[7];
	char Signal_Level;
	char Signal_Channel[3];
	char Channel_char;
	char IO_Init_Status;	//IO��ʼ״̬
	char ver;				//�Ż����汾��
	int validNum;			//��ǰ��Ч̨��
	int curSequence;		//������ѵ������
	
	float life_energy;			//ϵͳ��ʷ�ܵ���
	float current_energy;		//ϵͳ��ǰһ�ֵ���
	float today_energy;			//����ķ�����
	int system_power;			//ϵͳ�ܹ���
	int lastCommNum;
	
}ecu_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
