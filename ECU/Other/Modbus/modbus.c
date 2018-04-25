#include "modbus.h"
#include "usart485.h"
#include <rtthread.h>
#include "stdio.h"

#define MODBUS_DEBUG

#define MAX_SEND_TRAME_SIZE          256
#define MAX_RECV_TRAME_SIZE          4096
#define READ_MODBUS_TIMEOUT	    1000		//modbus每次读等待时间ms

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Sends a read N output word request                                      */
/*                                                                           */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   slaveNumber[in]:       slave number                                     */
/*   address[in]:           word address                                     */
/*   nbWords[in]:           number of words                                  */
/*   nbReadWords[out]:      number in bytes of read data                     */
/*   pWords[out]:           read data buffer                                 */
/*                                                                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   true - operation OK                                                     */
/*   false - operation failure                                               */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*             1 Byte 1 Byte   2 Bytes      2 Bytes      2 Bytes             */
/*            +------+------+------------+------------+------------+         */
/* Question   |Slave |Funct |    Word    | Number     |  CRC 16    |         */
/*            |Number| $03  |   Address  | of Words   |            |         */
/*            +------+------+------------+------------+------------+         */
/*                                                                           */
/*             1 Byte 1 Byte 1 Byte       2n Bytes              2 Bytes      */
/*            +------+------+------+------------ .... -------+------------+  */
/* Response   |Slave |Funct |Bytes | Words Value             |   CRC 16   |  */
/*            |Number| $03  |Count | 1        ------>   N    |            |  */
/*            +------+------+------+------------ .... -------+------------+  */
/*                                                                           */
/*****************************************************************************/
int read_N_OutWords(unsigned char slaveNumber, unsigned short address,
                    unsigned short nbWords, unsigned char* nbReadWords,
                    unsigned short* pWords)
{
    int nResend = 3;
    size_t i = 0,j = 0;
    unsigned char sendBuffer[MAX_SEND_TRAME_SIZE];
    size_t nbBytesToSend = 0;
    unsigned short CRC = 0,CRC_calc = 0,CRC_read = 0,CRClen = 0;

    sendBuffer[nbBytesToSend++] = slaveNumber;
    sendBuffer[nbBytesToSend++] = FC_READ_N_OUTWORDS;
    sendBuffer[nbBytesToSend++] = (address >> 8);
    sendBuffer[nbBytesToSend++] = (address & 0xFF);
    sendBuffer[nbBytesToSend++] = (nbWords >> 8);
    sendBuffer[nbBytesToSend++] = (nbWords & 0xFF);

    CRC = computeCRC(sendBuffer, nbBytesToSend);

    sendBuffer[nbBytesToSend++] = (CRC >> 8);
    sendBuffer[nbBytesToSend++] = (CRC & 0xFF);

#if defined(MODBUS_DEBUG)
    for(i=0; i<nbBytesToSend; i++)
    {
        printf("% 02x ",sendBuffer[i]);
    }
    printf("\n");
#endif

    // resend to avoid communication error
    while(nResend-- > 0)
    {
        clear_485();
        //发送数据
        Send485Data((char*)sendBuffer, nbBytesToSend);
        //读取数据

        // read response slave number and function code
        for(i = 0;i<(READ_MODBUS_TIMEOUT/10);i++)
        {
            if(USART2Cur == (2*nbWords + 5))
            {
                CRClen = 2*nbWords+3;
                //打印显示结果
#if defined(MODBUS_DEBUG)
                for(i=0; i<USART2Cur; i++)
                {
                    printf("% 02x ",USART2_RX_BUF[i]);
                }
                printf("\n");
#endif
                //判断Slave 以及Funct是否正确
                if((USART2_RX_BUF[0] != slaveNumber) || (USART2_RX_BUF[1] != FC_READ_N_OUTWORDS))
                {
                    return -2;
                }
                CRC_calc = computeCRC(USART2_RX_BUF, CRClen);
                CRC_read = ((((unsigned short)USART2_RX_BUF[CRClen] << 8) & 0xff00)
                            | ((unsigned short)USART2_RX_BUF[CRClen +1] & 0x00ff));

                *nbReadWords = USART2_RX_BUF[2];
                if(CRC_calc == CRC_read)
                {
                    for(j=0; j<(*nbReadWords); j++)
                    {
                        pWords[j] = ((((unsigned short)USART2_RX_BUF[ (j * 2)+3] << 8) & 0xff00)
                                     | ((unsigned short)USART2_RX_BUF[ (j * 2) + 4] & 0x00ff));
                    }
                    clear_485();
                    return 0;
                }else
                {
                    clear_485();
                    break;
                }

            }
            rt_thread_delay(RT_TICK_PER_SECOND/100);
        }

    }

    return -1;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Sends a read N input word request                                       */
/*                                                                           */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   slaveNumber[in]:       slave number                                     */
/*   address[in]:           word address                                     */
/*   nbWords[in]:           number of words                                  */
/*   nbReadWords[out]:      number in bytes of read data                     */
/*   pWords[out]:           read data buffer                                 */
/*                                                                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   true - operation OK                                                     */
/*   false - operation failure                                               */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*             1 Byte 1 Byte   2 Bytes      2 Bytes      2 Bytes             */
/*            +------+------+------------+------------+------------+         */
/* Question   |Slave |Funct |    Word    | Number     |  CRC 16    |         */
/*            |Number| $04  |   Address  | of Words   |            |         */
/*            +------+------+------------+------------+------------+         */
/*                                                                           */
/*             1 Byte 1 Byte 1 Byte       2n Bytes              2 Bytes      */
/*            +------+------+------+------------ .... -------+------------+  */
/* Response   |Slave |Funct |Bytes | Words Value             |   CRC 16   |  */
/*            |Number| $04  |Count | 1        ------>   N    |            |  */
/*            +------+------+------+------------ .... -------+------------+  */
/*                                                                           */
/*****************************************************************************/
int read_N_InWords(unsigned char slaveNumber, unsigned short address, 
                   unsigned short nbWords, unsigned char* nbReadWords,
                   unsigned short* pWords)
{
    int nResend = 3;
    size_t i = 0,j = 0;
    unsigned char sendBuffer[MAX_SEND_TRAME_SIZE];
    size_t nbBytesToSend = 0;
    unsigned short CRC = 0,CRC_calc = 0,CRC_read = 0,CRClen = 0;

    sendBuffer[nbBytesToSend++] = slaveNumber;
    sendBuffer[nbBytesToSend++] = FC_READ_N_INWORDS;
    sendBuffer[nbBytesToSend++] = (address >> 8);
    sendBuffer[nbBytesToSend++] = (address & 0xFF);
    sendBuffer[nbBytesToSend++] = (nbWords >> 8);
    sendBuffer[nbBytesToSend++] = (nbWords & 0xFF);

    CRC = computeCRC(sendBuffer, nbBytesToSend);

    sendBuffer[nbBytesToSend++] = (CRC >> 8);
    sendBuffer[nbBytesToSend++] = (CRC & 0xFF);

#if defined(MODBUS_DEBUG)
    for(i=0; i<nbBytesToSend; i++)
    {
        printf("% 02x ",sendBuffer[i]);
    }
    printf("\n");
#endif
    // resend to avoid communication error
    while(nResend-- > 0)
    {
        clear_485();
        //发送数据
        Send485Data((char*)sendBuffer, nbBytesToSend);
        //读取数据

        // read response slave number and function code
        for(i = 0;i<(READ_MODBUS_TIMEOUT/10);i++)
        {
            if(USART2Cur == (2*nbWords + 5))
            {
                CRClen = 2*nbWords+3;
                //打印显示结果
#if defined(MODBUS_DEBUG)
                for(i=0; i<USART2Cur; i++)
                {
                    printf("% 02x ",USART2_RX_BUF[i]);
                }
                printf("\n");
#endif
                //判断Slave 以及Funct是否正确
                if((USART2_RX_BUF[0] != slaveNumber) || (USART2_RX_BUF[1] != FC_READ_N_INWORDS))
                {
                    return -2;
                }
                CRC_calc = computeCRC(USART2_RX_BUF, CRClen);
                CRC_read = ((((unsigned short)USART2_RX_BUF[CRClen] << 8) & 0xff00)
                            | ((unsigned short)USART2_RX_BUF[CRClen +1] & 0x00ff));

                *nbReadWords = USART2_RX_BUF[2];
                if(CRC_calc == CRC_read)
                {
                    for(j=0; j<(*nbReadWords); j++)
                    {
                        pWords[j] = ((((unsigned short)USART2_RX_BUF[ (j * 2)+3] << 8) & 0xff00)
                                     | ((unsigned short)USART2_RX_BUF[ (j * 2) + 4] & 0x00ff));
                    }
                    clear_485();
                    return 0;
                }else
                {
                    clear_485();
                    break;
                }

            }
            rt_thread_delay(RT_TICK_PER_SECOND/100);
        }

    }

    return -1;
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Compute the CRC value                                                   */
/*                                                                           */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   pByte[in]:             data buffer to compute on                        */
/*   nbBytes[in]:           data in bytes                                    */
/*                                                                           */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   the CRC value                                                           */
/*                                                                           */
/*****************************************************************************/
unsigned short computeCRC(unsigned char* pByte, size_t nbBytes)
{
    unsigned short CRC = 0xFFFF;
    size_t count = 0;
    size_t bitCount = 0;
    unsigned char buff = 0;
    for(count = 0; count < nbBytes; count++)
    {
        CRC ^= pByte[count];
        for(bitCount = 0; bitCount != 8; bitCount++)
        {
            if(CRC & 1)
            {
                CRC >>= 1;
                CRC ^= 0xA001;
            }
            else
            {
                CRC >>= 1;
            }
        }
    }

    buff = (CRC >> 8);
    CRC <<= 8;
    CRC |= buff;

    return CRC;
}



#ifdef RT_USING_FINSH
#include <finsh.h>

void testModbus03(unsigned char slaveNumber,unsigned short address,unsigned char len)
{
    unsigned char nbReadWords;
    unsigned short pWords[256];
    int i = 0;
    if(read_N_OutWords(slaveNumber, address-1,len, &nbReadWords,pWords) < 0)
		return;
    printf("nbReadWords:%d \n",nbReadWords);
    for(i = 0;i<(nbReadWords/2);i++)
    {
        printf("%02x %02x ",(unsigned char)(pWords[i]/256),(unsigned char)(pWords[i]%256));
    }
    printf("\n");
}

void testModbus04(unsigned char slaveNumber,unsigned short address,unsigned char len)
{
    unsigned char nbReadWords;
    unsigned short pWords[256];
    int i = 0;
    if(read_N_InWords(slaveNumber, address-1,len, &nbReadWords,pWords)<0)
		return;
    printf("nbReadWords:%d \n",nbReadWords);
    for(i = 0;i<(nbReadWords/2);i++)
    {
        printf("%02x %02x ",(unsigned char)(pWords[i]/256),(unsigned char)(pWords[i]%256));
    }
    printf("\n");
}
FINSH_FUNCTION_EXPORT(testModbus03 , testModbus03.)
FINSH_FUNCTION_EXPORT(testModbus04 , testModbus04.)

#endif
