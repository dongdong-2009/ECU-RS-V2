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
#include "file.h"
#include <unistd.h>
#include "rtthread.h"
#include "debug.h"
#include "serverfile.h"

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
    if (FLASH_ErasePage(i) != FLASH_COMPLETE)
    {
      /* Error occurred while page erase */
      return (1);
    }
  }
  
  return (0);
}

#if 0
#ifdef RT_USING_FINSH
#include <finsh.h>
int copytoapp2(char *filename)
{
	FLASH_Unlock();
	FLASH_If_Erase_APP2();
	return FLASH_IF_FILE_COPY_TO_APP2(filename);
}

FINSH_FUNCTION_EXPORT(copytoapp2, eg:copytoapp2("/filename"));
#endif
#endif


