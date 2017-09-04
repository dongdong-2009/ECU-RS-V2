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
//地址
#define  ADDRESS_Test 							0x000000			//用于板子收发测试    			10字节
#define  ADDRESS_RebootNum					0x00000a			//用于记录重启次数					4个字节
#define  ADDRESS_ECUID 							0x000010			//ECU-RS ID   需要我们转换	6字节
#define  ADDRESS_CHANNEL 						0x000020			//优化器配置信息--信道			2字节
#define  ADDRESS_IO_INIT_STATUS 		0x000030			//IO上电初始化状态					1字节
#define  ADDRESS_UID_NUM 						0x000040			//优化器数量								2字节
#define  ADDRESS_WIFI_PW 						0x000050			//WIFI密码									100字节
#define  ADDRESS_UID 								0x000100			//优化器ID									6字节一个优化器
#define  ADDRESS_UID_BIND 					0x000100			//优化器ID 绑定标志					
#define  ADDRESS_UID_CHANNEL				0x000100			//优化器ID 信道							

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int Write_Test(char *Data_Ptr,unsigned char Counter);				//测试
int Read_Test(char *Data_Ptr,unsigned char Counter);

int Write_ECUID(char *ECUID);													  		//ECU ID
int Read_ECUID(char *ECUID);

//将6位ECU ID转换为12位ECU ID
void transformECUID(char * ECUID6,char *ECUID12);

int Write_CHANNEL(char *Channel);														//信道
int Read_CHANNEL(char *Channel);

int Write_IO_INIT_STATU(char *IO_InitStatus);								//IO上电状态
int Read_IO_INIT_STATU(char *IO_InitStatus);

int Write_WIFI_PW(char *WIFIPasswd,unsigned char Counter);	//WIFI密码
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
