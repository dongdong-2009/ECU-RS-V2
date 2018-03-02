#ifndef __APP_FUNCTION_H__
#define __APP_FUNCTION_H__

void App_GetBaseInfo(int Data_Len,const char *recvbuffer);
void App_GetSystemInfo(int Data_Len,const char *recvbuffer);
void App_GetPowerCurve(int Data_Len,const char *recvbuffer);
void App_GetGenerationCurve(int Data_Len,const char *recvbuffer);
void App_SetNetwork(int Data_Len,const char *recvbuffer);
void App_SetTime(int Data_Len,const char *recvbuffer);
void App_SetWiredNetwork(int Data_Len,const char *recvbuffer);
void App_SetWIFIPasswd(int Data_Len,const char *recvbuffer);
void App_GetIDInfo(int Data_Len,const char *recvbuffer);
void App_GetTime(int Data_Len,const char *recvbuffer) ;
void App_GetECUHardwareStatus(int Data_Len,const char *recvbuffer);
void App_SetChannel(int Data_Len,const char *recvbuffer);
void App_SetIOInitStatus(int Data_Len,const char *recvbuffer);
void APP_GetRSDHistoryInfo(int Data_Len,const char *recvbuffer);
void App_SetWiredNetwork(int Data_Len,const char *recvbuffer);
void App_GetWiredNetwork(int Data_Len,const char *recvbuffer);
void App_GetFlashSize(int Data_Len,const char *recvbuffer);
void APP_GetShortAddrInfo(int Data_Len,const char *recvbuffer);
void APP_GetECUAPInfo(int Data_Len,const char *recvbuffer) ;			//获取ECU连接AP信息
void APP_SetECUAPInfo(int Data_Len,const char *recvbuffer); 			//设置ECU连接AP
void APP_ListECUAPInfo(int Data_Len,const char *recvbuffer); 			//列举ECU 查询到的AP信息
void APP_GetFunctionStatusInfo(int Data_Len,const char *recvbuffer);
#endif	/*__APP_FUNCTION_H__*/

