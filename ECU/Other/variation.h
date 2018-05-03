/*****************************************************************************/
/* File      : variation.h                                                   */
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
#define MAXINVERTERCOUNT 						120	//�����������
#define INVERTERLENGTH 							56	//�����������  //���ֻ�ͨѶ

#define RECORDLENGTH                            150	//�Ӽ�¼�ĳ���
#define RECORDTAIL                              100	//���¼�Ľ�β��������������ʱ�����Ϣ

//Client ���ͨ�Ų���
#define CLIENT_RECORD_HEAD						20
#define CLIENT_RECORD_ECU_HEAD					78
#define CLIENT_RECORD_INVERTER_LENGTH           104
#define CLIENT_RECORD_OTHER						100
#define CLIENT_RECORD_JSON						10000

#define CONTROL_RECORD_HEAD						18
#define CONTROL_RECORD_ECU_HEAD					33
#define CONTROL_RECORD_INVERTER_LENGTH          41
#define CONTROL_RECORD_OTHER                    100


#define CONTROL_RECORD_ALARM_ECU_HEAD           (14*99+36)

#define JSON_RECORD_HEAD						200
#define JSON_RECORD_PER_INFO					200

#define MAX_THIRD_INVERTER_COUNT                20 //�����������������
#define MAX_THIRD_INVERTERID_LEN                32 //�����������������
#define MAX_CHANNEL_NUM 16         //���ͨ����
#define MAX_FACTORY_LEN 10         //��󹤳����Ƴ���
#define MAX_TYPE_LEN 10            //���������ͺų���
#define MAX_PV_NUM 	6
#define MAX_AC_NUM	3

#pragma pack(push)  
#pragma pack(1) 

typedef struct
{
    unsigned short comm_failed3_status:1;			//ͨѶ״̬ :  1 ����ͨѶ   0 ��������ͨѶ����
    unsigned short function_status:1;	//���ܿ���״̬: 1 ��    0 ��
    unsigned short pv1_low_voltage_pritection:1;	// PV1Ƿѹ����
    unsigned short pv2_low_voltage_pritection:1;	// PV2Ƿѹ����
    unsigned short device_Type:2;					//�豸����  0:�����豸 1������豸
    unsigned short comm_status:1;					//1��ʾ������ǰ���ݣ�0��ʾ��ȡ����ʧ��
    unsigned short bindflag:1;					//������󶨶̵�ַ��־��1��ʾ�󶨣�0��ʾδ��
    unsigned short flag:1;					//id�е�flag��־
    unsigned short mos_status:1;			//MOS��״̬
    unsigned short last_mos_status:1;		//���һ��MOS��״̬
    unsigned short last_function_status:1;	//���ܿ���״̬: 1 ��    0 ��
    unsigned short last_pv1_low_voltage_pritection:1;	// PV1Ƿѹ����
    unsigned short last_pv2_low_voltage_pritection:1;	// PV2Ƿѹ����
    unsigned short collect_ret:1;
    unsigned short turn_on_collect_data:1;	//����ʱ���ļ����Ƿ��ж�ӦRSD����(����Ϊ��һ�ֹػ�ǰ�洢����)
}status_t;

typedef struct
{
    unsigned char rsd_config_status:1;			//RSD����״̬: 1 RSD����ʹ��   0 RSD���ܽ���
    unsigned char Reserve1:7;				//����
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
    unsigned char model;			//���ͣ�00δ֪ 01�Ż��� 02 �ض��� 03JBOX
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
    unsigned char last_RSDTimeout;		//��һ��RSD��ʱʱ��
    unsigned char RSDTimeout;			//RSD��ʱʱ��
    unsigned short PV1_low_voltageNUM;	//PV1Ƿѹ����
    unsigned short PV2_low_voltageNUM;	//PV2Ƿѹ����
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

}ecu_info;

typedef struct
{
    unsigned char inverter_addr_flag:1;			//������Ƿ��Ѱ󶨵�ַ 1����  0����
    unsigned char autoget_addr:1;               			//�Ƿ��Զ���ȡ��ַ  1����  0����
    unsigned char communication_flag:1;			//��ǰһ��ͨѶ��־ 0:δͨѶ�� 1�Ѿ�ͨѶ��
    unsigned char Reserve1:5;				//����
}inverter_third_status_t;




//������������ṹ��
typedef struct inverter_third_info_t{
    char inverterid[MAX_THIRD_INVERTERID_LEN];//�����ID ��󳤶���32	OK
    unsigned char  inverter_addr;             //�������ַ       Modbus�ӻ���ַ
    char factory[MAX_FACTORY_LEN];            //�������������
    char type[MAX_TYPE_LEN];                  //������ͺ�						OK
    inverter_third_status_t third_status;     //����״̬


    float PV_Voltage[MAX_PV_NUM];                       //ֱ����ѹ		���6·
    float PV_Current[MAX_PV_NUM];                       //ֱ������		���6·
    float PV_Power[MAX_PV_NUM];                       //ֱ������		���6·
    float AC_Voltage[MAX_AC_NUM];                       //������ѹ		3���ѹ
    float AC_Current[MAX_AC_NUM];                       //��������		3���ѹ
    float Grid_Frequency[MAX_AC_NUM];                      //����Ƶ��
    float Temperature;                           //�����¶�
    int Reactive_Power;                        //�޹�����
    int Active_Power;                          //�й�����
    float Power_Factor;                        //��������
    float Daily_Energy;                        //�շ�����
    float Life_Energy;                         //��ʷ������
    float Current_Energy;                      //���ַ�����	//�շ���������

    int (*GetData_ThirdInverter)(struct inverter_third_info_t *curThirdinverter);
}inverter_third_info;


#pragma pack(pop) 

#endif /*__VARIATION_H__*/
