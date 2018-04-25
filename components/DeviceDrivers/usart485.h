/*****************************************************************************/
/* File      : 485usart.h                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2018-01-17 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

//ע�⿪������ʹ�õ�485���ӵ�TX RX���£�
//	PB10(USART3_TX)
//	PA3(USART2_RX)
//	ʹ�õ�������USART�Ŀ�
#ifndef __485_USART_H
#define __485_USART_H

#define USART2_REC_LEN 4096

extern unsigned char USART2_RX_BUF[USART2_REC_LEN];     			
extern unsigned short USART2Cur;	
void clear_485(void);
//��ʼ��485�˿�
void usart485_init(unsigned int bound);
int Send485Data(char *data, int num);
#endif
