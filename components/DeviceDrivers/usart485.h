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

//注意开发板中使用的485连接的TX RX如下：
//	PB10(USART3_TX)
//	PA3(USART2_RX)
//	使用的是两个USART的口
#ifndef __485_USART_H
#define __485_USART_H

#define USART2_REC_LEN 4096

extern unsigned char USART2_RX_BUF[USART2_REC_LEN];     			
extern unsigned short USART2Cur;	
void clear_485(void);
//初始化485端口
void usart485_init(unsigned int bound);
int Send485Data(char *data, int num);
#endif
