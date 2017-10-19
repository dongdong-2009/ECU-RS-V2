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
#define MAXINVERTERCOUNT 								100	//最大的逆变器数
#define INVERTERLENGTH 									56	//最大的逆变器数  //与手机通讯
//Client 相关通信参数
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
    unsigned short comm_failed3_status:1;			//通讯状态 :  1 正常通讯   0 连续三次通讯不上
	unsigned short function_status:1;	//功能开关状态: 1 开    0 关
	unsigned short heart_Failed_times:3; // 连续通信失败次数  ，当大于3的时候默认该RSD2为关机状态
	unsigned short pv1_low_voltage_pritection:1;	// PV1欠压保护
	unsigned short pv2_low_voltage_pritection:1;	// PV2欠压保护
	unsigned short device_Type:4;					//设备类型  0:开关设备 1；监控设备
	unsigned short comm_status:1;					//1表示读到当前数据；0表示读取数据失败
	unsigned short dataflag:1;					//采集数据通讯状态 用于采集发电量相关
	unsigned short bindflag:1;					//逆变器绑定短地址标志，1表示绑定，0表示未绑定
	unsigned short flag:1;					//id中的flag标志
	unsigned short unused:2;						//未使用变量  备用
}status_t;


typedef struct inverter_info_t{
	char uid[13];		//逆变器ID（在通讯的时候转换为BCD编码）
	unsigned short shortaddr;	//Zigbee的短地址
	int model;					//机型：1是YC250CN,2是YC250NA，3是YC500CN，4是YC500NA，5是YC900CN，6是YC900NA			
	int zigbee_version;					//zigbee版本号ZK			
	
	unsigned short version;				//软件版本号
	unsigned short heart_rate;	//心跳次数
	unsigned short off_times;	//心跳超时次数
	status_t status;			//部分状态信息 
	unsigned char restartNum;	//一天内的重启次数
	unsigned short PV1;		//PV1输入电压  精度 0.1V
	unsigned short PV2;		//PV2输入电压  精度 0.1V
	unsigned short PI;		//输入电流 	精度0.1A
	unsigned short PI2;		//输入电流 	精度0.1A
	
	unsigned short PV_Output; //输出电压 精度0.1V
	unsigned short PI_Output; //输出电流 	精度0.1A
	unsigned short Power1;	//PV1输入功率  精度0.1W
	unsigned short Power2;	//PV2输入功率  精度0.1W 
	unsigned short Power_Output;	//输出功率  精度0.1W 
	unsigned char RSSI;	//信号强度
	unsigned int PV1_Energy;//当前一轮PV1发电量	精度 1焦耳
	unsigned int PV2_Energy;//当前一轮PV2发电量	精度 1焦耳
	unsigned int PV_Output_Energy;//当前一轮PV2发电量	精度 1焦耳
	unsigned char Mos_CloseNum;//设备上电后MOS管关断次数
	char LastCommTime[15];	//RSD最后一次通讯上的时间	
	
	//上一轮相关的数据，这里的上一轮指的是5分钟一轮
	char LastCollectTime[15];	//上一轮采集时，最后一次通讯时间
	unsigned int Last_PV1_Energy;//上一轮PV1发电量 指的是5分钟前的一轮
	unsigned int Last_PV2_Energy;//上一轮PV2发电量 指的是5分钟前的一轮
	unsigned int Last_PV_Output_Energy;//上一轮PV2发电量 指的是5分钟前的一轮
	double AveragePower1; //5分钟平均功率1
	double AveragePower2; //5分钟平均功率2
	double AveragePower_Output; //5分钟平均功率2
	unsigned int EnergyPV1;		//当前一轮电量	精度 1焦耳
	unsigned int EnergyPV2;		//当前一轮电量	精度 1焦耳
	unsigned int EnergyPV_Output;		//当前一轮电量	精度 1焦耳
	
}inverter_info;

typedef struct ecu_info_t{
	char ECUID12[13];
	char ECUID6[7];
	unsigned short panid;				//Zigbee的panid
	char channel;				//Zigbee信道
	char Signal_Level;		//信号强度
	char IO_Init_Status;	//IO初始状态
	int count;					//系统当前一轮有数据的逆变器数
	int validNum;			//当前有效台数
	int curSequence;		//心跳轮训机器号
	char MacAddress[7];			//ECU  MAC地址
	float life_energy;			//系统历史总电量
	float current_energy;		//系统当前一轮电量
	float today_energy;			//当天的发电量
	int system_power;			//系统总功率
	int lastCommNum;
	
}ecu_info;

#pragma pack(pop) 

#endif /*__VARIATION_H__*/
