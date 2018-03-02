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
void APP_GetECUAPInfo(int Data_Len,const char *recvbuffer) ;			//��ȡECU����AP��Ϣ
void APP_SetECUAPInfo(int Data_Len,const char *recvbuffer); 			//����ECU����AP
void APP_ListECUAPInfo(int Data_Len,const char *recvbuffer); 			//�о�ECU ��ѯ����AP��Ϣ
void APP_GetFunctionStatusInfo(int Data_Len,const char *recvbuffer);
#endif	/*__APP_FUNCTION_H__*/

