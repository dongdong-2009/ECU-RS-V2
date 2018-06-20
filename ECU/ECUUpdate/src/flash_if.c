/*****************************************************************************/
/* File      : flash_if.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-10 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "flash_if.h"
#include <unistd.h>
#include "rtthread.h"
#include "debug.h"
#include "serverfile.h"
#include "mcp1316.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

//从文件中读取数据并烧写到内部的Flash
uint32_t FLASH_IF_FILE_COPY_TO_APP2(char * updateFileName)
{
    unsigned int app2addr = 0x08080000;
    unsigned int filedata,count;
    int fd;
    fd = fileopen(updateFileName,O_RDONLY,0);
    if(fd < 0)
    {
        printmsg(ECU_DBG_UPDATE,"FILE  open error");
        return 666;
    }
    while (app2addr < 0x080FFFFF)
    {
        count = fileRead(fd,(char *)&filedata,4);

        if (FLASH_ProgramWord(app2addr, filedata) == FLASH_COMPLETE)
        {
            /* Check the written value */

            if ((*(uint32_t *)(app2addr)) != (uint32_t)filedata)
            {
                printmsg(ECU_DBG_UPDATE,"compare error");
                fileclose(fd);
                /* Flash content doesn't match SRAM content */
                return(2);
            }
            /* Increment FLASH destination address */
            app2addr += 4;
        }
        else
        {
            printmsg(ECU_DBG_UPDATE," FLASH_ProgramWord  error");
            fileclose(fd);
            /* Error occurred while writing data in Flash memory */
            return (1);
        }
        if(count < 4)
        {
            break;
        }
    }
    fileclose(fd);
    FLASH_Unlock();
    while(FLASH_COMPLETE != FLASH_ErasePage(0x08004000))
    {}
    while(FLASH_COMPLETE != FLASH_ProgramHalfWord(0x08004000, 1))
    {}

    printmsg(ECU_DBG_UPDATE,"Program success");
    return (0);
}

uint32_t FLASH_If_Erase_APP2()
{
    uint32_t UserStartPage = 0x08080000,PageCount = 256, i = 0;

    for(i = 0x08080000; i < (UserStartPage+PageCount*0x800); i += 0x800)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
        while (FLASH_ErasePage(i) != FLASH_COMPLETE)
        {
            /* Error occurred while page erase */
            return (1);
        }
        MCP1316_kickwatchdog();
    }

    return (0);
}

//写内部Flash	地址 ，数据，数据长度
int FLASH_If_WriteData(uint32_t Address,char *Data,int length)
{
    int remainingLen = length;
    unsigned int app2addr = Address;
    unsigned int filedata_u32,index = 0;
    unsigned short filedata_u16 = 0;
    if((Address+length) < 0x080FFFFF)
    {
        while(remainingLen > 0)
        {
            
            if(remainingLen >=4)	//大于4个字节直接按照4字节写入
            {
                memcpy(&filedata_u32,&Data[index],4);
                if (FLASH_ProgramWord(app2addr, filedata_u32) == FLASH_COMPLETE)
                {
                    /* Check the written value */
                    if ((*(uint32_t *)(app2addr)) != (uint32_t)filedata_u32)
                    {
                        printmsg(ECU_DBG_UPDATE,"compare error");
                        /* Flash content doesn't match SRAM content */
                        return(-1);
                    }
                    /* Increment FLASH destination address */
                    app2addr += 4;
                    index += 4;
                    remainingLen -= 4;
                }else
                {
                		return -2;
                }
            }else if((remainingLen > 0)&&(remainingLen<4))	// 1个字节到3个字节按照半字写入
            {
                memcpy(&filedata_u16,&Data[index],2);
                if (FLASH_ProgramHalfWord(app2addr, filedata_u16) == FLASH_COMPLETE)
                {
                    /* Check the written value */
                    if ((*(uint16_t *)(app2addr)) != (uint16_t)filedata_u16)
                    {
                        printmsg(ECU_DBG_UPDATE,"compare error");
                        /* Flash content doesn't match SRAM content */
                        return(-1);
                    }
                    /* Increment FLASH destination address */
                    app2addr += 2;
                    index += 2;
                    remainingLen -= 2;
                }else
                	{
                	return -2;
                	}
            }else	//0个字节结束
            {
                return 0;
            }
        }
        return index;
    }else
    {
        return -1;
    }

}

void UpdateFlag(void)
{
    FLASH_Unlock();
    while(FLASH_COMPLETE != FLASH_ErasePage(0x08004000))
    {}
    while(FLASH_COMPLETE != FLASH_ProgramHalfWord(0x08004000, 1))
    {}

    printmsg(ECU_DBG_UPDATE,"Program success");
}


#if 1
#ifdef RT_USING_FINSH
#include <finsh.h>
int copytoapp2(char *filename)
{
    FLASH_Unlock();
    FLASH_If_Erase_APP2();
    return FLASH_IF_FILE_COPY_TO_APP2(filename);
}
int flashwrite(uint32_t Address,char *Data,int length)
{
	
	return FLASH_If_WriteData(Address,Data,length);
}
int flasherase(void)
{
	return FLASH_If_Erase_APP2();
}

void flashunlock(void)
{
	FLASH_Unlock();
}

void flashlock(void)
{
	FLASH_Lock();
}
FINSH_FUNCTION_EXPORT(flashwrite, eg:flashwrite("/filename"));
FINSH_FUNCTION_EXPORT(flasherase, eg:flasherase());
FINSH_FUNCTION_EXPORT(flashunlock, eg:flashunlock());
FINSH_FUNCTION_EXPORT(flashlock, eg:flashlock());
FINSH_FUNCTION_EXPORT(copytoapp2, eg:copytoapp2("/filename"));
#endif
#endif


