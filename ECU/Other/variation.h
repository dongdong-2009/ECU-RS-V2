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
#define MAXINVERTERCOUNT 100	//最大的逆变器数
#define INVERTERLENGTH 22	//最大的逆变器数
#pragma pack(push)  
#pragma pack(1) 

typedef struct
{
    unsigned short bind_status:1;		// 绑定状态  
    unsigned short mos_status:1;			//开关机状态 :  1 开    0 关
	unsigned short function_status:1;	//功能开关状态: 1 开    0 关
	unsigned short heart_Failed_times:3; // 连续通信失败次数  ，当大于3的时候默认该RSD2为关机状态
	unsigned short pv1_low_voltage_pritection:1;	// PV1欠压保护
	unsigned short pv2_low_voltage_pritection:1;	// PV2欠压保护
	unsigned short device_Type:4;					//设备类型  0:开关设备 1；监控设备
	unsigned short unused:4;						//未使用变量  备用
}status_t;


typedef struct inverter_info_t{
	unsigned char uid[6];		//逆变器ID（逆变器ID的BCD编码）
	unsigned short heart_rate;	//心跳次数
	unsigned short off_times;	//心跳超时次数
	status_t status;			//部分状态信息 
	unsigned char channel;		//信道状态
	unsigned char restartNum;	//一天内的重启次数
	unsigned short PV1;		//PV1输入电压  精度1V
	unsigned short PV2;		//PV2输入电压  精度 1V
	unsigned char PI;		//输入电流 	精度0.1A
	unsigned short Power1;	//PV1输入功率  精度1W
	unsigned short Power2;	//PV2输入功率  精度1W 
}inverter_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
