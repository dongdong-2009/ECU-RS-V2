#ifndef __MODBUS_H__
#define __MODBUS_H__
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2011-06-10 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Include Files                                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
#define FC_READ_N_OUTWORDS      0x03
#define FC_READ_N_INWORDS       0x04
typedef unsigned int size_t;

// function code 03
int read_N_OutWords(unsigned char slaveNumber, unsigned short address,
                    unsigned short nbWords, unsigned char* nbReadWords,
                    unsigned short* pWords);
// function code 04
int read_N_InWords(unsigned char slaveNumber, unsigned short address, 
                   unsigned short nbWords, unsigned char* nbReadWords,
                   unsigned short* pWords);

unsigned short computeCRC(unsigned char* pByte, size_t size);


#endif /*__MODBUS_H__*/
