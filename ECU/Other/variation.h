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
#define MAXINVERTERCOUNT 100	//�����������
#define INVERTERLENGTH 22	//�����������
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
	unsigned short unused:4;						//δʹ�ñ���  ����
}status_t;


typedef struct inverter_info_t{
	unsigned char uid[6];		//�����ID�������ID��BCD���룩
	unsigned short heart_rate;	//��������
	unsigned short off_times;	//������ʱ����
	status_t status;			//����״̬��Ϣ 
	unsigned char channel;		//�ŵ�״̬
	unsigned char restartNum;	//һ���ڵ���������
	unsigned short PV1;		//PV1�����ѹ  ����1V
	unsigned short PV2;		//PV2�����ѹ  ���� 1V
	unsigned char PI;		//������� 	����0.1A
	unsigned short Power1;	//PV1���빦��  ����1W
	unsigned short Power2;	//PV2���빦��  ����1W 
}inverter_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
