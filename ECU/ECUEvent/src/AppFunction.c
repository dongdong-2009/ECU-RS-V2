#include "AppFunction.h"
#include "event.h"
#include "string.h"
#include "usart5.h"
#include "appcomm.h"
#include "SEGGER_RTT.h"
#include "inverter.h"
#include "rthw.h"
#include "led.h"
#include "serverfile.h"
#include "version.h"
#include "debug.h"
#include <dfs_posix.h> 
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include <dfs_fs.h>
#include <dfs_file.h>
#include "rtc.h"
#include "threadlist.h"
#include "channel.h"
#include "rsdFunction.h"
#include "debug.h"
#include "third_inverter.h"
#include "zigbee.h"

extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern inverter_third_info thirdInverterInfo[MAX_THIRD_INVERTER_COUNT];

//返回0 表示格式正确，返回1表示发送格式错误
int strToHex(const char *recvbuff,unsigned char *buff,int length)
{
	int i =0;
	for(i = 0;i<length;i++)
	{
		if((recvbuff[i*2] >= 'a') && (recvbuff[i*2] <= 'f'))
		{
			buff[i] = (recvbuff[i*2] -'a' + 10)*0x10;
		}else if((recvbuff[i*2] >= 'A') && (recvbuff[i*2] <= 'F'))
		{
			buff[i] = (recvbuff[i*2] -'A' + 10)*0x10;
		}else if((recvbuff[i*2] >= '0') && (recvbuff[i*2] <= '9'))
		{
			buff[i] = (recvbuff[i*2] -'0' )*0x10;
		}else
		{
			return -1;
		}

		if((recvbuff[i*2+1] >= 'a') && (recvbuff[i*2+1] <= 'f'))
		{
			buff[i] += (recvbuff[i*2+1] -'a' + 10);
		}else if((recvbuff[i*2+1] >= 'A') && (recvbuff[i*2+1] <= 'F'))
		{
			buff[i] += (recvbuff[i*2+1] -'A' + 10);
		}else if((recvbuff[i*2+1] >= '0') && (recvbuff[i*2+1] <= '9'))
		{
			buff[i] += (recvbuff[i*2+1] -'0');
		}else
		{
			return -1;
		}
		
	}
	return 0;
}

int switchChannel(unsigned char *buff)
{
	int ret=0x17;
	if((buff[0]>='0') && (buff[0]<='9'))
			buff[0] -= 0x30;
	if((buff[0]>='A') && (buff[0]<='F'))
			buff[0] -= 0x37;
	if((buff[0]>='a') && (buff[0]<='f'))
			buff[0] -= 0x57;
	if((buff[1]>='0') && (buff[1]<='9'))
			buff[1] -= 0x30;
	if((buff[1]>='A') && (buff[1]<='F'))
			buff[1] -= 0x37;
	if((buff[1]>='a') && (buff[1]<='f'))
			buff[1] -= 0x57;
	ret = (buff[0]*16+buff[1]);
	return ret;
}

int ResolveServerInfo(const char *string,ECUServerInfo_t *serverInfo)
{
	char cmd[3] = {'\0'};
	unsigned char cmdNO = 0;
	char domainLenStr[4] = {'\0'};
	unsigned char domainLen = 0;
	
	memcpy(cmd,&string[25],2);
	cmdNO = atoi(cmd);
	if((cmdNO<=0) ||(cmdNO>=7))
		return -1;
	serverInfo->serverCmdType = (eServerCmdType)atoi(cmd);
	if((serverInfo->serverCmdType == SERVER_UPDATE_GET)||(serverInfo->serverCmdType == SERVER_CLIENT_GET)||(serverInfo->serverCmdType == SERVER_CONTROL_GET))
		return 0;
	memcpy(domainLenStr,&string[30],3);
	domainLen = atoi(domainLenStr);
	memcpy(serverInfo->domain,&string[33],domainLen);

	memcpy(serverInfo->IP,&string[33+domainLen],4);

	serverInfo->Port1 = string[37+domainLen]*256 + string[38+domainLen];
	serverInfo->Port2 = string[39+domainLen]*256 + string[40+domainLen];
	return 0;
	
}

int Save_Server(ECUServerInfo_t *serverInfo)
{	
	char *buff = NULL;
	buff = malloc(512);
	memset(buff,'\0',512);
	printf("%d\n",serverInfo->serverCmdType);
	printf("%s\n",serverInfo->domain);
	printf("%d,%d,%d,%d,\n",serverInfo->IP[0],serverInfo->IP[1],serverInfo->IP[2],serverInfo->IP[3]);
	printf("%d\n",serverInfo->Port1);
	printf("%d\n",serverInfo->Port2);
	if(serverInfo->serverCmdType == SERVER_UPDATE_SET)
	{
		sprintf(buff,"Domain=%s\nIP=%d.%d.%d.%d\nPort=%d\nuser=zhyf\npassword=yuneng\n",serverInfo->domain,serverInfo->IP[0],serverInfo->IP[1],serverInfo->IP[2],serverInfo->IP[3],serverInfo->Port1);
		echo("/config/ftpadd.con", buff);
	}else if(serverInfo->serverCmdType == SERVER_CLIENT_SET)
	{
		sprintf(buff,"Domain=%s\nIP=%d.%d.%d.%d\nPort1=%d\nPort2=%d\n",serverInfo->domain,serverInfo->IP[0],serverInfo->IP[1],serverInfo->IP[2],serverInfo->IP[3],serverInfo->Port1,serverInfo->Port2);
		echo("/config/datacent.con",buff);
	}else if(serverInfo->serverCmdType == SERVER_CONTROL_SET)
	{
		sprintf(buff,"Timeout=10\nReport_Interval=15\nDomain=%s\nIP=%d.%d.%d.%d\nPort1=%d\nPort2=%d\n",serverInfo->domain,serverInfo->IP[0],serverInfo->IP[1],serverInfo->IP[2],serverInfo->IP[3],serverInfo->Port1,serverInfo->Port2);
		echo("/config/control.con",buff);
	}
	free(buff);
	buff = NULL;
	
	return 0;
}

int phone_add_inverter(int num,const char *uidstring)
{
	int i = 0;
	char buff[25] = { '\0' };
	char *allbuff = NULL;
	allbuff = malloc(2500);
	memset(allbuff,'\0',2500);
	
	for(i = 0; i < num; i++)
	{
		memset(buff,'\0',25);
		sprintf(buff,"%02x%02x%02x%02x%02x%02x,,,,,,\n",uidstring[0+i*6],uidstring[1+i*6],uidstring[2+i*6],uidstring[3+i*6],uidstring[4+i*6],uidstring[5+i*6]);
		memcpy(&allbuff[0+19*i],buff,19);
	}
	printf("%s\n",allbuff);
	echo("/home/data/id",allbuff);
	echo("/config/limiteid.con","1");
	free(allbuff);
	allbuff = NULL;
	return 0;
}

int phone_add_thirdinverter(int num,const char *uidstring)
{
	int i = 0,length = 0,buff_len = 0;
	char buff[60] = { '\0' };
	char thirdID[32] = {'\0'};
	unsigned short address = 0;
	char thirdFactory[16] = {'\0'};
	char thirdType[16] = {'\0'};
	char cBaudRate = 0;
	char *allbuff = NULL;
	allbuff = malloc(2500);
	memset(allbuff,'\0',2500);
	
	for(i = 0; i < num; i++)
	{
		memcpy(thirdID,&uidstring[i*54],32);
		address = uidstring[i*54 +32];
		memcpy(thirdFactory,&uidstring[i*54 +33],10);
		memcpy(thirdType,&uidstring[i*54 +43],10);
		cBaudRate = uidstring[i*54 +53];

		memset(buff,'\0',60);
		sprintf(buff,"%s,%d,,%s,%s,,%d\n",thirdID,address,thirdFactory,thirdType, cBaudRate);
		buff_len = strlen(buff);
		memcpy(&allbuff[length],buff,buff_len);
		length +=buff_len;
	}
	printf("%s\n",allbuff);
	echo("/home/data/thirdid",allbuff);
	free(allbuff);
	allbuff = NULL;
	return 0;
}

int Resolve2Str(char *firstStr,int *firstLen,char *secondStr,int *secondLen,char *string)
{
	*firstLen = (string[0]-'0')*10+(string[1]-'0');
	memcpy(firstStr,&string[2],*firstLen);
	*secondLen = (string[2+(*firstLen)]-'0')*10+(string[3+(*firstLen)]-'0');
	memcpy(secondStr,&string[4+*firstLen],*secondLen);
	
	return 0;
}

//??0 ????IP  ??1 ????IP   ??-1????
int ResolveWired(const char *string,IP_t *IPAddr,IP_t *MSKAddr,IP_t *GWAddr,IP_t *DNS1Addr,IP_t *DNS2Addr)
{
	if(string[1] == '0')	//动态IP
	{
		return 0;
	}else if(string[1] == '1') //静态IP
	{
		IPAddr->IP1 = string[2];
		IPAddr->IP2 = string[3];
		IPAddr->IP3 = string[4];
		IPAddr->IP4 = string[5];
		MSKAddr->IP1 = string[6];
		MSKAddr->IP2 = string[7];
		MSKAddr->IP3 = string[8];
		MSKAddr->IP4 = string[9];
		GWAddr->IP1 = string[10];
		GWAddr->IP2 = string[11];
		GWAddr->IP3 = string[12];
		GWAddr->IP4 = string[13];	
		DNS1Addr->IP1 = string[14];	
		DNS1Addr->IP2 = string[15];	
		DNS1Addr->IP3 = string[16];	
		DNS1Addr->IP4 = string[17];	
		DNS2Addr->IP1 = string[18];	
		DNS2Addr->IP2 = string[19];	
		DNS2Addr->IP3 = string[20];	
		DNS2Addr->IP4 = string[21];	
		return 1;
	}else
	{
		return -1;
	}

}

void App_GetBaseInfo(int Data_Len,const char *recvbuffer) 				//获取基本信息请求
{
	char sofewareVersion[50];
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 1 ",(char *)recvbuffer);
	
	sprintf(sofewareVersion,"%s_%s.%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	APP_Response_BaseInfo(ecu,ECU_VERSION_LENGTH,sofewareVersion,inverterInfo);
}

void App_GetSystemInfo(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 2 ",(char *)recvbuffer);
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		APP_Response_SystemInfo(0x00,inverterInfo,ecu.validNum);
	}	else
	{	//不匹配
		APP_Response_SystemInfo(0x01,inverterInfo,0);
	}

}

void App_GetPowerCurve(int Data_Len,const char *recvbuffer)
{
	char date[9];
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 3 ",(char *)recvbuffer);
	memset(date,'\0',9);
	
	memcpy(date,&recvbuffer[28],8);
	//匹配成功进行相应操作
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_PowerCurve(0x00,date);
	}else
	{
		APP_Response_PowerCurve(0x01,date);
	}

}

void App_GetGenerationCurve(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 4 ",(char *)recvbuffer);

	//匹配成功进行相应操作
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GenerationCurve(0x00,recvbuffer[29]);
	}else
	{
		APP_Response_GenerationCurve(0x01,recvbuffer[29]);
	}
}


void App_SetNetwork(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 5 ",(char *)recvbuffer);
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		int AddNum = 0;
		int i = 0;
		inverter_info *curinverter = inverterInfo;
		AddNum = (WIFI_Recv_SocketA_LEN - 31)/6;
		printf("num:%d\n",AddNum);
		APP_Response_SetNetwork(0x00);
		//将数据写入EEPROM
		phone_add_inverter(AddNum,(char *)&recvbuffer[28]);	
		
		for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
		{
			rt_memset(curinverter->uid, '\0', sizeof(curinverter->uid));		//清空逆变器UID
		}
		
		get_id_from_file(inverterInfo);
		//重启main线程
		restartThread(TYPE_DATACOLLECT);
	}	else
	{	//不匹配
		APP_Response_SetNetwork(0x01);
	}

}

void App_SetTime(int Data_Len,const char *recvbuffer) 			//ECU时间设置
{
	char setTime[15];
	char getTime[15];
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 6 ",(char *)recvbuffer);

	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		//匹配成功进行相应操作
		memset(setTime,'\0',15);
		memcpy(setTime,&recvbuffer[28],14);
		apstime(getTime);
		if(!memcmp("99999999",setTime,8))
		{	//如果年月日都是9 年月日取当前的
			memcpy(setTime,getTime,8);
		}

		if(!memcmp("999999",&setTime[8],6))
		{	//如果时间都是9 时间取当前的
			memcpy(&setTime[8],&getTime[8],6);
		}
			
		//设置时间
		set_time(setTime);
		//重启main线程
		restartThread(TYPE_DATACOLLECT);	
		APP_Response_SetTime(0x00);
	}
	else
	{
		APP_Response_SetTime(0x01);
	}
	

}


void App_SetWiredNetwork(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 7 ",(char *)recvbuffer);	

	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		int ModeFlag = 0;
		char buff[200] = {'\0'};
		IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
		//检查是DHCP  还是固定IP
		APP_Response_SetWiredNetwork(0x00);
		ModeFlag = ResolveWired(&recvbuffer[28],&IPAddr,&MSKAddr,&GWAddr,&DNS1Addr,&DNS2Addr);
		if(ModeFlag == 0x00)		//DHCP
		{
			printmsg(ECU_DBG_WIFI,"dynamic IP");
			unlink("/config/staticIP.con");
			dhcp_reset();
		}else if (ModeFlag == 0x01)		//固定IP		
		{
			printmsg(ECU_DBG_WIFI,"static IP");
			//保存网络地址
			sprintf(buff,"IPAddr=%d.%d.%d.%d\nMSKAddr=%d.%d.%d.%d\nGWAddr=%d.%d.%d.%d\nDNS1Addr=%d.%d.%d.%d\nDNS2Addr=%d.%d.%d.%d\n",IPAddr.IP1,IPAddr.IP2,IPAddr.IP3,IPAddr.IP4,
			MSKAddr.IP1,MSKAddr.IP2,MSKAddr.IP3,MSKAddr.IP4,GWAddr.IP1,GWAddr.IP2,GWAddr.IP3,GWAddr.IP4,DNS1Addr.IP1,DNS1Addr.IP2,DNS1Addr.IP3,DNS1Addr.IP4,DNS2Addr.IP1,DNS2Addr.IP2,DNS2Addr.IP3,DNS2Addr.IP4);
			echo("/config/staticIP.con",buff);
			//设置固定IP
			StaticIP(IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);
		}
						
		
	}	
	else
	{
		APP_Response_SetWiredNetwork(0x01);
	}

}

void App_GetECUHardwareStatus(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 8 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		APP_Response_GetECUHardwareStatus(0x00);
	}else
	{
		APP_Response_GetECUHardwareStatus(0x01);
	}
}


void App_SetWIFIPasswd(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 10 ",(char *)recvbuffer);
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		char OldPassword[100] = {'\0'};
		char NewPassword[100] = {'\0'};
		char EEPROMPasswd[100] = {'\0'};
		int oldLen,newLen;
				
		Resolve2Str(OldPassword,&oldLen,NewPassword,&newLen,(char *)&recvbuffer[28]);								
								
		//读取旧密码，如果旧密码相同，设置新密码
		Read_WIFI_PW(EEPROMPasswd,100);
				
		if(!memcmp(EEPROMPasswd,OldPassword,oldLen))
		{
			APP_Response_SetWifiPassword(0x00);
			WIFI_ChangePasswd(NewPassword);
			Write_WIFI_PW(NewPassword,newLen);
		}else
		{
			APP_Response_SetWifiPassword(0x02);
		}
								
	}	else
	{	//不匹配
		APP_Response_SetWifiPassword(0x01);
	}

}

void App_GetIDInfo(int Data_Len,const char *recvbuffer) 			//获取ID信息
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 11 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GetIDInfo(0x00,inverterInfo);
	}else
	{
		APP_Response_GetIDInfo(0x01,inverterInfo);
	}
	
}

void App_GetTime(int Data_Len,const char *recvbuffer) 			//获取时间
{
	char Time[15] = {'\0'};
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 12 ",(char *)recvbuffer);
	apstime(Time);
	Time[14] = '\0';
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GetTime(0x00,Time);
	}else
	{
		APP_Response_GetTime(0x01,Time);
	}

}

void App_GetFlashSize(int Data_Len,const char *recvbuffer)
{
	int result;
	long long cap;
	struct statfs buffer;

	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 13",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		result = dfs_statfs("/", &buffer);
		if (result != 0)
		{
			APP_Response_FlashSize(0x00,0);
			return;
		}
		cap = buffer.f_bsize * buffer.f_bfree / 1024;

		APP_Response_FlashSize(0x00,(unsigned int)cap);
	}else
	{
		APP_Response_FlashSize(0x01,0);
	}


}

void App_GetWiredNetwork(int Data_Len,const char *recvbuffer)
{
	unsigned int addr = 0;
	extern struct netif *netif_list;
	struct netif * netif;
	int ModeFlag = 0;
	ip_addr_t DNS;
	IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;

	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 14",(char *)recvbuffer);  

	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		netif = netif_list;
		addr = ip4_addr_get_u32(&netif->ip_addr);
		IPAddr.IP4 = (addr/16777216)%256;
		IPAddr.IP3 = (addr/65536)%256;
		IPAddr.IP2 = (addr/256)%256;
		IPAddr.IP1 = (addr/1)%256;
		  
		addr = ip4_addr_get_u32(&netif->netmask);
		MSKAddr.IP4 = (addr/16777216)%256;
		MSKAddr.IP3 = (addr/65536)%256;
		MSKAddr.IP2 = (addr/256)%256;
		MSKAddr.IP1 = (addr/1)%256;
		  
		addr = ip4_addr_get_u32(&netif->gw);
		GWAddr.IP4 = (addr/16777216)%256;
		GWAddr.IP3 = (addr/65536)%256;
		GWAddr.IP2 = (addr/256)%256;
		GWAddr.IP1 = (addr/1)%256;
		  
		DNS = dns_getserver(0);
		addr = ip4_addr_get_u32(&DNS);
		DNS1Addr.IP4 = (addr/16777216)%256;
		DNS1Addr.IP3 = (addr/65536)%256;
		DNS1Addr.IP2 = (addr/256)%256;
		DNS1Addr.IP1 = (addr/1)%256;
		
		DNS = dns_getserver(1);
		addr = ip4_addr_get_u32(&DNS);
		DNS2Addr.IP4 = (addr/16777216)%256;
		DNS2Addr.IP3 = (addr/65536)%256;
		DNS2Addr.IP2 = (addr/256)%256;
		DNS2Addr.IP1 = (addr/1)%256;
		
		//检查是DHCP	还是固定IP
		ModeFlag = get_DHCP_Status();
			  
		APP_Response_GetWiredNetwork(0x00,ModeFlag,IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr,ecu.MacAddress);
		
	}else
	{
		APP_Response_GetWiredNetwork(0x01,ModeFlag,IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr,ecu.MacAddress);
	}

}



void App_SetChannel(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 15 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		unsigned char old_channel = 0x00;
		unsigned char new_channel = 0x00;
	
		APP_Response_SetChannel(0x00,ecu.channel,ecu.Signal_Level);
		old_channel = switchChannel((unsigned char *)&recvbuffer[28]);
		new_channel = switchChannel((unsigned char *)&recvbuffer[30]);	

		saveOldChannel(old_channel);
		saveNewChannel(new_channel);
		saveChannel_change_flag();
		//重启main线程
		restartThread(TYPE_DATACOLLECT);
	}	else
	{	//不匹配
		APP_Response_SetChannel(0x01,NULL,NULL);
	}

}


void App_SetIOInitStatus(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 16 ",(char *)recvbuffer);
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{//匹配成功进行相应的操作
		
		
		//0：低电平（关闭心跳功能）
		//1：高电平（打开心跳功能）
		if(recvbuffer[25] == '0'){
			APP_Response_IOInitStatus(0x00);
			save_rsdFunction_change_flag();
			saveChangeFunctionStatus(2,0,0);//关闭心跳功能
			unlink("/tmp/setrsd");
			unlink("/tmp/rsdcon.con");
			//重启main线程
			restartThread(TYPE_DATACOLLECT);
		}else if(recvbuffer[25] == '1'){
			APP_Response_IOInitStatus(0x00);
			save_rsdFunction_change_flag();
			saveChangeFunctionStatus(1,2,0);//打开心跳功能
			unlink("/tmp/setrsd");
			unlink("/tmp/rsdcon.con");
			//重启main线程
			restartThread(TYPE_DATACOLLECT);
		}else if(recvbuffer[25] == '2'){	//单台设置RSD功能
			int num = (Data_Len -32)/7;
			APP_Response_IOInitStatus(0x00);
	
			insertAppSetRSDInfo(num,(char *)&recvbuffer[29]);
			insertAppRSDCon(num,(char *)&recvbuffer[29]);
			//重启main线程
			restartThread(TYPE_DATACOLLECT);
		}else if(recvbuffer[25] == '3'){	//查询当前RSD状态
			APP_Response_IOInitStatus(0x02);
		}else
		{
			;
		}
		
		
	}else
	{
		APP_Response_IOInitStatus(0x01);
	}

}

void APP_GetRSDHistoryInfo(int Data_Len,const char *recvbuffer)
{
	char date[9] = {'\0'};
	char UID[7] = {'\0'};
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 17 ",(char *)recvbuffer);

	memcpy(date,&recvbuffer[28],8);
	date[8] = '\0';
	memcpy(UID,&recvbuffer[36],6);
	
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GetRSDHistoryInfo(0x00,date,UID);

	}else
	{
		APP_Response_GetRSDHistoryInfo(0x01,date,UID);

	}
}


void APP_GetShortAddrInfo(int Data_Len,const char *recvbuffer) 			//获取ID信息
{
	
	
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 18 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GetShortAddrInfo(0x00,inverterInfo);
	}else
	{
		APP_Response_GetShortAddrInfo(0x01,inverterInfo);
	}
	
}

void APP_GetECUAPInfo(int Data_Len,const char *recvbuffer) 			//获取ECU连接AP信息
{
	unsigned char connectStatus = 0;
	char info[100] = {'\0'};
	
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 20 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		connectStatus = AT_CWJAPStatus(info);
		printf("connectStatus:%d\n",connectStatus);
		APP_Response_GetECUAPInfo(0x00,connectStatus,info);
	}else
	{
		APP_Response_GetECUAPInfo(0x01,connectStatus,info);
	}
	
}

void APP_SetECUAPInfo(int Data_Len,const char *recvbuffer) 			//设置ECU连接AP
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 21 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		//匹配成功进行相应的操作
		char SSID[100] = {'\0'};
		char Password[100] = {'\0'};

		int ssidLen,passwdLen;
				
		Resolve2Str(SSID,&ssidLen,Password,&passwdLen,(char *)&recvbuffer[30]);								
		printf("SSID:  %s   ,PASSWD: %s \n",SSID,Password);					

		APP_Response_SetECUAPInfo(0x00);
		AT_CWJAP(SSID,Password);
	}else
	{
		APP_Response_SetECUAPInfo(0x01);
	}
	
}

void APP_ListECUAPInfo(int Data_Len,const char *recvbuffer) 			//列举ECU 查询到的AP信息
{
	unsigned char ret = 0;
	char *list = NULL;

	list = malloc(4096);
	memset(list,'\0',4096);
	
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 22 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		ret = AT_CWLAPList(list);
		if(0 == ret)
		{
			APP_Response_GetECUAPList(0x00,list);
		}else
		{
			APP_Response_GetECUAPList(0x00,"");
		}
		
	}else
	{
		APP_Response_GetECUAPList(0x01,list);
	}
	free(list);
	list = NULL;
	
}


void APP_GetFunctionStatusInfo(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 23 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		APP_Response_GetFunctionStatusInfo(0x00);
	}else
	{
		APP_Response_GetFunctionStatusInfo(0x01);
	}
}

void APP_ServerInfo(int Data_Len,const char *recvbuffer) 			
{
	int ret = 0;
	ECUServerInfo_t serverInfo;
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 24 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		//判断是具体哪条命令
		ret = ResolveServerInfo(recvbuffer,&serverInfo);
		if(ret == 0)
		{	
			APP_Response_ServerInfo(0x00,&serverInfo);
		}else
		{
			return;
		}	
	}else
	{
		APP_Response_ServerInfo(0x01,&serverInfo);
	}
	
}

void APP_RegisterThirdInverter(int Data_Len,const char *recvbuffer)
{
	int cmd = 0;
	char cmd_str[3] = {'\0'};
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 33 ",(char *)recvbuffer);
	memcpy(cmd_str,&recvbuffer[25],2);
	cmd_str[2] = '\0';
	cmd = atoi(cmd_str);
	
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		//判断是注册命令还是读取命令
		
		if(cmd == 1)
		{
			
			int AddNum = 0;
			int i = 0;
			inverter_third_info *curThirdinverter = thirdInverterInfo;
			AddNum = (WIFI_Recv_SocketA_LEN - 33)/54;
			printf("num:%d\n",AddNum);
			
			APP_Response_RegisterThirdInverter(cmd,0x00);
			
			//将数据写入EEPROM
			phone_add_thirdinverter(AddNum,(char *)&recvbuffer[30]);	
			
			for(i=0; i<MAX_THIRD_INVERTER_COUNT; i++, curThirdinverter++)
			{
				rt_memset(curThirdinverter->inverterid, '\0', sizeof(curThirdinverter->inverterid));		//清空逆变器UID
			}
			
			get_ThirdID_from_file(thirdInverterInfo);
			//重启main线程
			restartThread(TYPE_DATACOLLECT);
			
		}else if(cmd == 2)
		{
			APP_Response_RegisterThirdInverter(cmd,0x00);
		}else
		{
			return;
		}
		
		
	}	else
	{	//不匹配
		APP_Response_RegisterThirdInverter(cmd,0x01);
	}
	
}


void APP_GetThirdInverter(int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 34 ",(char *)recvbuffer);
	//先对比ECUID是否匹配
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{	//匹配成功进行相应的操作
		APP_Response_GetThirdInverter(0x00);
	}	else
	{	//不匹配
		APP_Response_GetThirdInverter(0x01);
	}

}

void APP_TransmissionZigBeeInfo(int Data_Len,const char *recvbuffer)
{
	char CommFlag = '0';
	unsigned char buff[256] = {'\0'};//需要发送的报文
	int length,i;
	inverter_info *curinverter = inverterInfo;
	char UID[12] = {'\0'};
	print2msg(ECU_DBG_EVENT,"WIFI_Recv_Event 35 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.ECUID12,12))
	{
		//解析是广播还是单播
		CommFlag = recvbuffer[28];
		memcpy(UID,&recvbuffer[29],12);
		memcpy(buff,&recvbuffer[41],3);
		buff[3] = '\0';
		length = atoi((char*)buff);
		if(-1 == strToHex(&recvbuffer[44],buff,length))
		{
			APP_Response_TransmissionZigBeeInfo(0x02);
			return;
		}
		
		if('0' == CommFlag)	//广播
		{
			zb_broadcast_cmd((char *)buff,length);
		}else			//单播
		{
			for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
			{
				if(!memcmp(curinverter->uid,UID,12))
				{
					zb_send_cmd(curinverter,(char *)buff,length);
				}
			}
		}
		APP_Response_TransmissionZigBeeInfo(0x00);
	}else
	{
		APP_Response_TransmissionZigBeeInfo(0x01);
	}
}

