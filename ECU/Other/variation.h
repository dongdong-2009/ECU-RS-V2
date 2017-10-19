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
#define INVERTERLENGTH 									56	//�����������  //���ֻ�ͨѶ
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
    unsigned short comm_failed3_status:1;			//ͨѶ״̬ :  1 ����ͨѶ   0 ��������ͨѶ����
	unsigned short function_status:1;	//���ܿ���״̬: 1 ��    0 ��
	unsigned short heart_Failed_times:3; // ����ͨ��ʧ�ܴ���  ��������3��ʱ��Ĭ�ϸ�RSD2Ϊ�ػ�״̬
	unsigned short pv1_low_voltage_pritection:1;	// PV1Ƿѹ����
	unsigned short pv2_low_voltage_pritection:1;	// PV2Ƿѹ����
	unsigned short device_Type:4;					//�豸����  0:�����豸 1������豸
	unsigned short comm_status:1;					//1��ʾ������ǰ���ݣ�0��ʾ��ȡ����ʧ��
	unsigned short dataflag:1;					//�ɼ�����ͨѶ״̬ ���ڲɼ����������
	unsigned short bindflag:1;					//������󶨶̵�ַ��־��1��ʾ�󶨣�0��ʾδ��
	unsigned short flag:1;					//id�е�flag��־
	unsigned short unused:2;						//δʹ�ñ���  ����
}status_t;


typedef struct inverter_info_t{
	char uid[13];		//�����ID����ͨѶ��ʱ��ת��ΪBCD���룩
	unsigned short shortaddr;	//Zigbee�Ķ̵�ַ
	int model;					//���ͣ�1��YC250CN,2��YC250NA��3��YC500CN��4��YC500NA��5��YC900CN��6��YC900NA			
	int zigbee_version;					//zigbee�汾��ZK			
	
	unsigned short version;				//����汾��
	unsigned short heart_rate;	//��������
	unsigned short off_times;	//������ʱ����
	status_t status;			//����״̬��Ϣ 
	unsigned char restartNum;	//һ���ڵ���������
	unsigned short PV1;		//PV1�����ѹ  ���� 0.1V
	unsigned short PV2;		//PV2�����ѹ  ���� 0.1V
	unsigned short PI;		//������� 	����0.1A
	unsigned short PI2;		//������� 	����0.1A
	
	unsigned short PV_Output; //�����ѹ ����0.1V
	unsigned short PI_Output; //������� 	����0.1A
	unsigned short Power1;	//PV1���빦��  ����0.1W
	unsigned short Power2;	//PV2���빦��  ����0.1W 
	unsigned short Power_Output;	//�������  ����0.1W 
	unsigned char RSSI;	//�ź�ǿ��
	unsigned int PV1_Energy;//��ǰһ��PV1������	���� 1����
	unsigned int PV2_Energy;//��ǰһ��PV2������	���� 1����
	unsigned int PV_Output_Energy;//��ǰһ��PV2������	���� 1����
	unsigned char Mos_CloseNum;//�豸�ϵ��MOS�ܹضϴ���
	char LastCommTime[15];	//RSD���һ��ͨѶ�ϵ�ʱ��	
	
	//��һ����ص����ݣ��������һ��ָ����5����һ��
	char LastCollectTime[15];	//��һ�ֲɼ�ʱ�����һ��ͨѶʱ��
	unsigned int Last_PV1_Energy;//��һ��PV1������ ָ����5����ǰ��һ��
	unsigned int Last_PV2_Energy;//��һ��PV2������ ָ����5����ǰ��һ��
	unsigned int Last_PV_Output_Energy;//��һ��PV2������ ָ����5����ǰ��һ��
	double AveragePower1; //5����ƽ������1
	double AveragePower2; //5����ƽ������2
	double AveragePower_Output; //5����ƽ������2
	unsigned int EnergyPV1;		//��ǰһ�ֵ���	���� 1����
	unsigned int EnergyPV2;		//��ǰһ�ֵ���	���� 1����
	unsigned int EnergyPV_Output;		//��ǰһ�ֵ���	���� 1����
	
}inverter_info;

typedef struct ecu_info_t{
	char ECUID12[13];
	char ECUID6[7];
	unsigned short panid;				//Zigbee��panid
	char channel;				//Zigbee�ŵ�
	char Signal_Level;		//�ź�ǿ��
	char IO_Init_Status;	//IO��ʼ״̬
	int count;					//ϵͳ��ǰһ�������ݵ��������
	int validNum;			//��ǰ��Ч̨��
	int curSequence;		//������ѵ������
	char MacAddress[7];			//ECU  MAC��ַ
	float life_energy;			//ϵͳ��ʷ�ܵ���
	float current_energy;		//ϵͳ��ǰһ�ֵ���
	float today_energy;			//����ķ�����
	int system_power;			//ϵͳ�ܹ���
	int lastCommNum;
	
}ecu_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
