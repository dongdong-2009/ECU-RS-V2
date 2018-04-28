/*
 * Created by zhyf
 * Created on 2016.07.06
 * Description: 此版本CRC校验用于ECU-C上，与Sensor的单片机通信
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define POLY 0x1021

unsigned short UpdateCRC(unsigned short CRC_acc, unsigned char CRC_input)
{
    unsigned short i;
    CRC_acc = CRC_acc ^ (CRC_input << 8);

    for (i = 0; i < 8; i++)
    {
        // Check if the MSB is set (if MSB is 1, then the POLY can "divide"
        // into the "dividend")
        if ((CRC_acc & 0x8000) == 0x8000)
        {
            // if so, shift the CRC value, and XOR "subtract" the poly
            CRC_acc = CRC_acc << 1;
            CRC_acc ^= POLY;
        }
        else
        {
            // if not, just shift the CRC value
            CRC_acc = CRC_acc << 1;
        }
    }

    return CRC_acc;
}

unsigned short crc_array(unsigned char *buff, int len)
{
    unsigned short result = 0xFFFF;
    int i = 0;

    for(i=0; i<len; i++)
        result = UpdateCRC(result, buff[i]);

    return result;
}

unsigned short crc_file(char *file)
{
    unsigned short result = 0xFFFF;
    char package_buff[128];
    int fd;

    fd = open(file, O_RDONLY,0);
    if(fd>=0)
    {
        while(read(fd, package_buff, 1)>0)
            result = UpdateCRC(result, package_buff[0]);
        close(fd);
    }

    return result;
}

unsigned short crc_16_opt(char *pData,int len,int init,int *ptable)
{
    unsigned short result = init;
    char temp;
    int i=0;

    while(len-- > 0)
    {
        temp=result>>8;
        result=(result<<8)^ptable[(temp^(pData[i++]))&0xFF];
    }

    return result;
}
