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
#define MAXINVERTERCOUNT 								120	//�����������
#define INVERTERLENGTH 									56	//�����������  //���ֻ�ͨѶ

#define RECORDLENGTH 150		//�Ӽ�¼�ĳ���
#define RECORDTAIL 100			//���¼�Ľ�β��������������ʱ�����Ϣ

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

#define JSON_RECORD_HEAD						200
#define JSON_RECORD_PER_INFO					200

#pragma pack(push)  
#pragma pack(1) 

typedef enum
{ 
    DEVICE_UNKNOWN    	= 0,		//δ֪�豸
    DEVICE_OPT700 		= 1,		//�Ż���
    DEVICE_OPT700_RS  	= 2,		//�ض���
    DEVICE_JBOX 		= 3		//Jbox
} eDeviceType;

typedef struct
{
    unsigned short comm_failed3_status:1;			//ͨѶ״̬ :  1 ����ͨѶ   0 ��������ͨѶ����
    unsigned short function_status:1;				//RSD���ܿ���״̬: 1 RSD���ܿ�    0 RSD���� ��
    unsigned short pv1_low_voltage_pritection:1;	// PV1Ƿѹ����(���Ż����Զ��ϱ�)
    unsigned short pv2_low_voltage_pritection:1;	// PV2Ƿѹ����(���Ż����Զ��ϱ�)
    unsigned short device_Type:2;				//�豸����  0:�����豸 1������豸
    unsigned short comm_status:1;				//1��ʾ������ǰ���ݣ�0��ʾ��ȡ����ʧ��
    unsigned short bindflag:1;					//������󶨶̵�ַ��־��1��ʾ�󶨣�0��ʾδ��
    unsigned short flag:1;						//id�е�flag��־
    unsigned char turn_on_off_flag:1;				//��ǰ��ȡ���Ŀ��ػ�״̬  0:�ػ� 1:����
    unsigned short alarm_flag:1;					//�Ƿ���Ҫ�ϱ��澯��־λ
    unsigned short collect_ret:1;
    unsigned short turn_on_collect_data:1;			//����ʱ���ļ����Ƿ��ж�ӦRSD����(����Ϊ��һ�ֹػ�ǰ�洢����)
    unsigned char pv2_alarm_status:2	;			//ECU��⵽OPT�澯״̬0: �ޱ��� 1:PV2�����ѹ����100V   2:PV2�����ѹΪ0V
    unsigned char Reserve1:1; 
}status_t;

typedef struct
{
    unsigned char rsd_config_status:1;			//RSD����״̬: 1 RSD����ʹ��   0 RSD���ܽ���
    unsigned char Reserve1:7;					//����
}config_status_t;

typedef struct
{
    unsigned short  Version_Control_flag:1;	//�汾����λ
    unsigned short  Reserve1:2;	//����λ
    unsigned short  Energy2_flag:1;	//����2��������־λ
    unsigned short  Energy1_flag:1;	//����1��������־λ
    unsigned short  EnergyOut_flag:1;	//�����������־λ
    unsigned short  Temperature_flag:1;	//�¶ȱ�־λ
    unsigned short  Current2_flag:1;	//PV2�������
    unsigned short  Current1_flag:1;	//PV1�������
    unsigned short  CurrentOut_flag:1;	//�������
    unsigned short  Power2_flag:1;	//PV2���빦��
    unsigned short  Power1_flag:1;	//PV1���빦��
    unsigned short  PowerOut_flag:1;	//�������
    unsigned short  Voltage2_flag:1;	//PV2�����ѹ
    unsigned short  Voltage1_flag:1;	//PV1�����ѹ
    unsigned short  VoltageOut_flag:1;	//�����ѹ
}parameter_status_t;

typedef struct inverter_info_t{
    char uid[13];				//�����ID����ͨѶ��ʱ��ת��ΪBCD���룩
    unsigned short shortaddr;		//Zigbee�Ķ̵�ַ
    unsigned char model;			//���ͣ�00δ֪ 
    							//                01�Ż��� 
    							//                02 �ض��� 
    							//                03JBOX
    int zigbee_version;			//zigbee�汾��ZK

    unsigned short version;		//����汾��
    unsigned short heart_rate;	//��������
    unsigned short off_times;		//������ʱ����
    status_t status;			//����״̬��Ϣ
    parameter_status_t parameter_status;	//RSD��ز�Ʒ���͹�����״̬
    config_status_t config_status;	//����״̬��Ϣ
    unsigned char restartNum;		//һ���ڵ���������

    unsigned char  temperature; 	//�豸�ڲ��¶� -100��Ϊ��ʵ�¶�

    unsigned short PV1;			//PV1�����ѹ  ���� 0.1V
    unsigned short PV2;			//PV2�����ѹ  ���� 0.1V
    unsigned short PI;			//������� 	����0.1A
    unsigned short PI2;			//������� 	����0.1A

    unsigned short PV_Output; 	//�����ѹ ����0.1V
    unsigned short PI_Output; 	//������� 	����0.1A
    unsigned short Power1;		//PV1���빦��  ����0.1W
    unsigned short Power2;		//PV2���빦��  ����0.1W
    unsigned short Power_Output;	//�������  ����0.1W
    unsigned char RSSI;			//�ź�ǿ��
    unsigned int PV1_Energy;		//��ǰһ��PV1������	���� 1����
    unsigned int PV2_Energy;		//��ǰһ��PV2������	���� 1����
    unsigned int PV_Output_Energy;//��ǰһ��PV2������	���� 1����
    unsigned char Mos_CloseNum;	//�豸�ϵ��MOS�ܹضϴ���
    unsigned char RSDTimeout;			//RSD��ʱʱ��
    unsigned short PV1_low_voltageNUM;	//PV1Ƿѹ����
    unsigned short PV2_low_voltageNUM;	//PV2Ƿѹ����
    unsigned short PV2_low_differenceNUM;//PV2����Ƿѹ��ֵ
    char LastCommTime[15];			//RSD���һ��ͨѶ�ϵ�ʱ��

    //��һ����ص����ݣ��������һ��ָ����5����һ��
    char LastCollectTime[15];	//��һ�ֲɼ�ʱ�����һ��ͨѶʱ��
    unsigned int Last_PV1_Energy;//��һ��PV1������ ָ����5����ǰ��һ��
    unsigned int Last_PV2_Energy;//��һ��PV2������ ָ����5����ǰ��һ��
    unsigned int Last_PV_Output_Energy;//��һ��PV2������ ָ����5����ǰ��һ��
    double AveragePower1; 			//5����ƽ������1
    double AveragePower2; 			//5����ƽ������2
    double AveragePower_Output; 		//5����ƽ������2
    unsigned int EnergyPV1;			//��ǰһ�ֵ���	���� 1����
    unsigned int EnergyPV2;			//��ǰһ�ֵ���	���� 1����
    unsigned int EnergyPV_Output;		//��ǰһ�ֵ���	���� 1����
    unsigned char no_getdata_num;	//unsigned char(��������255)����û�л�ȡ����������ݵĴ���

}inverter_info;

typedef struct ecu_info_t{
    char ECUID12[13];
    char ECUID6[7];
    unsigned short panid;				//Zigbee��panid
    char channel;				//Zigbee�ŵ�
    char Signal_Level;		//�ź�ǿ��
    char IO_Init_Status;	//IO��ʼ״̬	'1'��ʹ��  '0���ǽ���
    int count;					//ϵͳ��ǰһ�������ݵ��������
    int validNum;			//��ǰ��Ч̨��
    int curSequence;		//�ɼ���ѵ������
    int curHeartSequence;   //������ѵ������
    char MacAddress[7];			//ECU  MAC��ַ
    float life_energy;			//ϵͳ��ʷ�ܵ���
    float current_energy;		//ϵͳ��ǰһ�ֵ���
    float today_energy;			//����ķ�����
    int system_power;			//ϵͳ�ܹ���
    int lastCommNum;
    char curTime[15];			//���һ�βɼ���ʱ��
    char JsonTime[15];			//���һ�βɼ���ʱ��
    unsigned char flag_ten_clock_getshortaddr;	//ÿ��10����û�����»�ȡ�̵�ַ��־
    int polling_total_times;			//ECUһ��֮���ܵ���ѯ���� ZK
    unsigned char idUpdateFlag;		//id���±�־
    unsigned char ThirdIDUpdateFlag;		//�����������id���±�־
    int thirdCommNum;
    int thirdCount;		//�������������̨��
    
    unsigned short abnormalNum;			//�����쳣��̨��
    unsigned int haveDataTimes;			//�����ݵ�����
    unsigned char faulttimes;			//���ϴ���
    unsigned int nextdetectionTimes;		//��һ�μ�����
    unsigned int overdetectionTimes;		//���һ�μ�����
}ecu_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
