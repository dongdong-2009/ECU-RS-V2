/*****************************************************************************/
/* File      : file.h                                                        */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-06-04 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#ifndef __FILE_H__
#define __FILE_H__
#include "Flash_24L512.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define VERSION_ECU_RS    					"101"
#define SOFEWARE_VERSION_LENGTH			5
#define SOFEWARE_VERSION						"RS1.3"
//��ַ
#define  ADDRESS_Test 							0x000000			//���ڰ����շ�����    			10�ֽ�
#define  ADDRESS_RebootNum					0x00000a			//���ڼ�¼��������					4���ֽ�
#define  ADDRESS_ECUID 							0x000010			//ECU-RS ID   ��Ҫ����ת��	6�ֽ�
#define  ADDRESS_CHANNEL 						0x000020			//�Ż���������Ϣ--�ŵ�			2�ֽ�
#define  ADDRESS_IO_INIT_STATUS 		0x000030			//IO�ϵ��ʼ��״̬					1�ֽ�
#define  ADDRESS_UID_NUM 						0x000040			//�Ż�������								2�ֽ�
#define  ADDRESS_WIFI_PW 						0x000050			//WIFI����									100�ֽ�
#define  ADDRESS_UID 								0x000100			//�Ż���ID									6�ֽ�һ���Ż���
#define  ADDRESS_UID_BIND 					0x000100			//�Ż���ID �󶨱�־					
#define  ADDRESS_UID_CHANNEL				0x000100			//�Ż���ID �ŵ�							

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int Write_Test(char *Data_Ptr,unsigned char Counter);				//����
int Read_Test(char *Data_Ptr,unsigned char Counter);

int Write_ECUID(char *ECUID);													  		//ECU ID
int Read_ECUID(char *ECUID);

//��6λECU IDת��Ϊ12λECU ID
void transformECUID(char * ECUID6,char *ECUID12);

int Write_CHANNEL(char *Channel);														//�ŵ�
int Read_CHANNEL(char *Channel);

int Write_IO_INIT_STATU(char *IO_InitStatus);								//IO�ϵ�״̬
int Read_IO_INIT_STATU(char *IO_InitStatus);

int Write_WIFI_PW(char *WIFIPasswd,unsigned char Counter);	//WIFI����
int Read_WIFI_PW(char *WIFIPasswd,unsigned char Counter);

int Write_UID_NUM(char *UID_NUM);														//UID_NUM
int Read_UID_NUM(char *UID_NUM);

int Write_UID(char *UID,int n);															//UID
int Read_UID(char *UID,int n);			

int Write_UID_Bind(char BindFlag,int n);
int Read_UID_Bind(char *BindFlag,int n);

int Write_UID_Channel(char channel,int n);
int Read_UID_Channel(char *channel,int n);

int Write_rebootNum(unsigned int num);
int Read_rebootNum(unsigned int *num);


#endif /*__FILE_H__*/
