#include "InternalFlash.h"
#include <rtthread.h>
#include <stdio.h>
#include "stm32f10x.h"
#include <unistd.h>
#include "rtthread.h"

//????
int ErasePage(eInternalFlashType type)
{
	if((type != INTERNAL_FALSH_Update) && (type != INTERNAL_FALSH_ID) && (type != INTERNAL_FALSH_MAC) && (type != INTERNAL_FALSH_AREA))
		return -1;
	FLASH_Unlock();
	if(type == INTERNAL_FALSH_Update)
	{
		while(FLASH_COMPLETE != FLASH_ErasePage(0x08004000))
		{}
		
	}else if(type == INTERNAL_FALSH_ID)
	{
		while(FLASH_COMPLETE != FLASH_ErasePage(0x08004800))
		{}
		
	}else if(type == INTERNAL_FALSH_MAC)
	{
		while(FLASH_COMPLETE != FLASH_ErasePage(0x08005000))
		{}
	}else if(type == INTERNAL_FALSH_AREA)
	{
		while(FLASH_COMPLETE != FLASH_ErasePage(0x08005800))
		{}
		
	}else
	{
		;
	}
	FLASH_Lock();
	return 0;
}

//????
int WritePage(eInternalFlashType type,char *Data,int Length)
{
	unsigned int addr = 0;
	int i = 0;
	unsigned short sData = 0;
	if(0 == ErasePage(type))
	{
		if(type == INTERNAL_FALSH_Update)
		{
			addr = 0x08004000;
		}else if(type == INTERNAL_FALSH_ID)
		{
			addr = 0x08004800;
		}else if(type == INTERNAL_FALSH_MAC)
		{
			addr = 0x08005000;	
		}else if(type == INTERNAL_FALSH_AREA)
		{
			addr = 0x08005800;	
		}else
		{
			return -4;
		}
		FLASH_Unlock();
		
		for(i = 0;i < Length/2+(Length%2);i++)
		{
			sData = Data[i*2]*0x100 + Data[i*2+1];
			if(FLASH_ProgramHalfWord(addr, sData) == FLASH_COMPLETE)
			{
				if ((*(uint16_t *)(addr)) != (uint16_t)sData)
				{		
					/* Flash content doesn't match SRAM content */
					return(-3);
				}
			}
			else
			{
				return (-2);
			}
			addr += 2;
		}
		FLASH_Lock();
		return 0;
	}else
	{
		return -1;
	}
	
}
//????
int ReadPage(eInternalFlashType type,char *Data,int Length)
{
    unsigned int addr = 0;
	int i = 0;
	unsigned short sData = 0;
	if(type == INTERNAL_FALSH_Update)
	{
		addr = 0x08004000;
	}else if(type == INTERNAL_FALSH_ID)
	{
		addr = 0x08004800;
	}else if(type == INTERNAL_FALSH_MAC)
	{
		addr = 0x08005000;	
	}else if(type == INTERNAL_FALSH_AREA)
	{
		addr = 0x08005800;	
	}else
	{
		return -1;
	}
	
	for(i = 0;i < Length/2+(Length%2);i++)
	{
		sData = (*(uint16_t *)(addr));
		Data[i*2] = sData/0x100;
		Data[i*2 + 1] = sData%0x100;
		addr+=2;	
	}
	return 0;
}

void detectionInternalFlash(char *ID,unsigned char *Mac)
{
	char temp[18] = {0x00};
	ReadPage(INTERNAL_FALSH_ID,temp,12);	//??ID,?????FF,??ID
	temp[12] = '\0';
	if((temp[0] == 0xFF)&&(temp[1] == 0xFF)&&(temp[2] == 0xFF)&&(temp[3] == 0xFF)&&(temp[4] == 0xFF)&&(temp[5] == 0xFF)&&
		(temp[6] == 0xFF)&&(temp[7] == 0xFF)&&(temp[8] == 0xFF)&&(temp[9] == 0xFF)&&(temp[10] == 0xFF)&&(temp[11] == 0xFF))
	{
		WritePage(INTERNAL_FALSH_ID,ID,12);
	}

	ReadPage(INTERNAL_FALSH_MAC,temp,17);	//??ID,?????FF,??ID
	temp[17] = '\0';
	if((temp[0] == 0xFF)&&(temp[1] == 0xFF)&&(temp[2] == 0xFF)&&(temp[3] == 0xFF)&&(temp[4] == 0xFF)&&(temp[5] == 0xFF)&&
		(temp[6] == 0xFF)&&(temp[7] == 0xFF)&&(temp[8] == 0xFF)&&(temp[9] == 0xFF)&&(temp[10] == 0xFF)&&(temp[11] == 0xFF)&&
		(temp[12] == 0xFF)&&(temp[13] == 0xFF)&&(temp[14] == 0xFF)&&(temp[15] == 0xFF)&&(temp[16] == 0xFF))
	{
		sprintf(temp,"%02X:%02X:%02X:%02X:%02X:%02X",Mac[0],Mac[1],Mac[2],Mac[3],Mac[4],Mac[5]);
		WritePage(INTERNAL_FALSH_MAC,temp,17);
	}
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void ReadP(eInternalFlashType type,int Length)
{
	char Data[20] = {'\0'};
	ReadPage(type,Data,Length);
	Data[Length] = '\0';
	printf("%s\n",Data);

}

FINSH_FUNCTION_EXPORT(ErasePage, eg:ErasePage());
FINSH_FUNCTION_EXPORT(WritePage, eg:WritePage());
FINSH_FUNCTION_EXPORT(ReadP, eg:ReadP());
#endif
