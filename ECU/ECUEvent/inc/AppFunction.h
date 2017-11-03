#ifndef __APP_FUNCTION_H__
#define __APP_FUNCTION_H__

void App_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetSystemInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer) ;

void App_SetChannel(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetIOInitStatus(unsigned char * ID,int Data_Len,const char *recvbuffer);
void APP_GetRSDHistoryInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);
void App_GetFlashSize(unsigned char * ID,int Data_Len,const char *recvbuffer);
void APP_GetShortAddrInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);


#endif	/*__APP_FUNCTION_H__*/

