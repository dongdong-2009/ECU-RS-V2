#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"


extern signed char COMM_Timeout_Event;

//WIFI���ڽ��ճ�ʱ�ж�
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Deinit(void); 

//�������ڽ��ճ�ʱ�ж�
void TIM4_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Deinit(void); 

//1S��ʱ�� 
void TIM2_Int_Init(u16 arr,u16 psc);	//	TIM2_Int_Init(9999,7199);    1S����һ���ж�
void TIM2_Int_Deinit(void); 
void TIM2_Refresh(void);



#endif
