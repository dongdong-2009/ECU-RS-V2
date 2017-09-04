#include <stm32f10x.h>
#include <rtthread.h>
#include "Flash_24L512.h"
//============================================================================
#define EEPROM_RCC              	RCC_APB2Periph_GPIOB
#define SCL_GPIO                	GPIOB
#define SCL_PIN                 	(GPIO_Pin_0)

#define SDA_GPIO                	GPIOB
#define SDA_PIN										(GPIO_Pin_1)

#define WP_GPIO                   GPIOB
#define WP_PIN                		(GPIO_Pin_10)

#define  SDA_1    GPIO_SetBits(SDA_GPIO, SDA_PIN)
#define  SDA_0    GPIO_ResetBits(SDA_GPIO, SDA_PIN)
#define  SCL_1    GPIO_SetBits(SCL_GPIO, SCL_PIN)
#define  SCL_0    GPIO_ResetBits(SCL_GPIO, SCL_PIN)
#define  DIR_IN   GPIO_SetBits(SDA_GPIO, SDA_PIN)
#define  DIR_OUT  GPIO_ResetBits(SDA_GPIO, SDA_PIN)
#define  SDA_IN  (GPIO_ReadInputDataBit(SDA_GPIO, SDA_PIN) & 0x01)

#define  TIME     15
#define  Clr_EEPROM_WP      GPIO_ResetBits(WP_GPIO, WP_PIN)
#define  Set_EEPROM_WP      GPIO_SetBits(WP_GPIO, WP_PIN)
#define  Write_24L256_Ctrl  0xA0
#define  Read_24L256_Ctrl   0xA1
//============================================================================
void I2CDelay(unsigned int n)
{
    unsigned int i;
    for(i = 0;i < n;i++);
}
//============================================================================
void I2CStart(void)
{
	DIR_OUT;
	SDA_1;
	SCL_1;
	I2CDelay(TIME);
	SDA_0;
	I2CDelay(TIME);
	SCL_0;
}
//============================================================================
void I2CStop(void)
{
	DIR_OUT;
	SDA_0;
	I2CDelay(TIME);
	SCL_1;
	I2CDelay(TIME);
	SDA_1;
}
//============================================================================
void I2CSendByte(unsigned char ucWRData)
{
	unsigned int i;

	DIR_OUT;
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		I2CDelay(TIME);
		if((ucWRData >> 7) & 0x01) 
            SDA_1;
		else 
            SDA_0;
		I2CDelay(TIME);
		SCL_1;
		I2CDelay(TIME);
		I2CDelay(TIME);
		ucWRData <<= 1;
	}
	SCL_0;
	SDA_1;
	I2CDelay(TIME);
}
//============================================================================
void I2CReceiveACK(void)
{
	unsigned char i=0 ;

	SCL_1;
	I2CDelay(TIME);
	DIR_IN;
	//while(SDA_IN)
	while((SDA_IN)&&(i<255))
	{
		i++;
	}
	DIR_OUT;
	SCL_0;
	I2CDelay(TIME);
}
//============================================================================
unsigned char I2CReceiveByte(void)
{
	unsigned char i;
	unsigned char ucRDData = 0;
	unsigned char ucDataBit = 0;

	SCL_0;
	I2CDelay(TIME);
	SDA_1;
	I2CDelay(TIME);
	DIR_IN;
	for(i = 0;i < 8;i++)
	{
		SCL_1;
		I2CDelay(TIME);
		ucDataBit = SDA_IN;
		I2CDelay(TIME);
		ucRDData = ((ucRDData << 1) | ucDataBit);
		SCL_0;
		I2CDelay(TIME);
	}
	return(ucRDData);
}
//============================================================================
void I2CAcknowledge(void)
{
	SCL_0;
	DIR_OUT;
	SDA_0;
	I2CDelay(TIME);
	SCL_1;
	I2CDelay(TIME);
	SCL_0;
}
//============================================================================
void Write_24L512_Byte(unsigned int ucWRAddress,unsigned char uiWRData)
{
	unsigned char ucLowAddress;
	unsigned char ucHighAddress;
	unsigned char ucData;

	ucLowAddress = (unsigned char)(ucWRAddress & 0x00ff);
	ucHighAddress = (unsigned char)(ucWRAddress >>8);
	ucData = uiWRData;
	Clr_EEPROM_WP;
	I2CStart();
	I2CSendByte(Write_24L256_Ctrl);
	I2CReceiveACK();
	I2CSendByte(ucHighAddress);
	I2CReceiveACK();
	I2CSendByte(ucLowAddress);
	I2CReceiveACK();
	I2CSendByte(ucData);
	I2CReceiveACK();
	I2CStop();
	I2CDelay(45000);
	Set_EEPROM_WP;
	I2CDelay(1000);
}
//============================================================================
void Write_24L512_nByte(unsigned int ucWRAddress,unsigned char Counter,unsigned char *Data_Ptr)
{
	unsigned char ucLowAddress;
	unsigned char ucHighAddress;
	unsigned char *p;
	unsigned char DataCounter;

	ucLowAddress = (unsigned char)(ucWRAddress & 0x00ff);
	ucHighAddress = (unsigned char)(ucWRAddress >>8);
	p = Data_Ptr;
	DataCounter = Counter;
	Clr_EEPROM_WP;
	I2CStart();
	I2CSendByte(Write_24L256_Ctrl);
	I2CReceiveACK();
	I2CSendByte(ucHighAddress);
	I2CReceiveACK();
	I2CSendByte(ucLowAddress);
	I2CReceiveACK();
	while(DataCounter)
	{
		I2CSendByte(*p);
		I2CReceiveACK();
		p++;
		DataCounter--;
	}
	I2CStop();
	I2CDelay(45000);
	Set_EEPROM_WP;
	I2CDelay(1000);
}
//============================================================================
unsigned char Read_24L512_Byte(unsigned int ucRDAddress)
{
	unsigned char ucLowAddress;
	unsigned char ucHighAddress;
	unsigned char uiDataReg;

	ucLowAddress = (unsigned char)(ucRDAddress & 0x00ff);
	ucHighAddress = (unsigned char)(ucRDAddress >>8);
	I2CStart();
	I2CSendByte(Write_24L256_Ctrl);
	I2CReceiveACK();
	I2CSendByte(ucHighAddress);
	I2CReceiveACK();
	I2CSendByte(ucLowAddress);
	I2CReceiveACK();

	I2CStart();
	I2CSendByte(Read_24L256_Ctrl);
	I2CReceiveACK();
	uiDataReg = I2CReceiveByte();
	I2CStop();
	I2CDelay(1000);
	return (uiDataReg);
}
//============================================================================
void Read_24L512_nByte(unsigned int ucRDAddress,unsigned char Counter, unsigned char *Data_Ptr)
{
	unsigned char ucLowAddress;
	unsigned char ucHighAddress;
	unsigned char *p;
	unsigned char DataCounter;

	ucLowAddress = (unsigned char)(ucRDAddress & 0x00ff);
	ucHighAddress = (unsigned char)(ucRDAddress >>8);
	p = Data_Ptr;
	DataCounter = Counter;
	I2CStart();
	I2CSendByte(Write_24L256_Ctrl);
	I2CReceiveACK();
	I2CSendByte(ucHighAddress);
	I2CReceiveACK();
	I2CSendByte(ucLowAddress);
	I2CReceiveACK();

	I2CStart();
	I2CSendByte(Read_24L256_Ctrl);
	I2CReceiveACK();
	while(DataCounter != 0)
	{
		*p = I2CReceiveByte();
		if(DataCounter >1)
		{
			I2CAcknowledge();
		}
		p++;
		DataCounter--;
	}
	I2CStop();
	I2CDelay(1000);
}
//============================================================================
void I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(EEPROM_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = SCL_PIN;        //选中管脚1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz
	GPIO_Init(SCL_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SDA_PIN;        //选中管脚1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;      //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz
	GPIO_Init(SDA_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = WP_PIN;     //选中管脚2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz
	GPIO_Init(WP_GPIO, &GPIO_InitStructure);
}


#if 1
#ifdef RT_USING_FINSH
#include <finsh.h>
void EEPROMReadTest()
{
	unsigned char recvbuff[11];
	Read_24L512_nByte(0x000000,10,recvbuff);
	rt_kprintf("read date on address 0x000000:%s\n",recvbuff);
}
void EEPROMWriteTest()
{
	unsigned char sendbuff[11] = "YuNeng APS";
	Write_24L512_nByte(0x000000,10,sendbuff);
	rt_kprintf("write date on address 0x000000:%s\n",sendbuff);
}
void ReadE(unsigned int ucRDAddress,unsigned char Counter)
{
	unsigned char recvbuff[256];
	Read_24L512_nByte(ucRDAddress,Counter,recvbuff);
	rt_kprintf("read date on address 0x%06x:%s\n",ucRDAddress,recvbuff);
}

void WriteE(unsigned int ucRDAddress,unsigned char Counter, unsigned char *Data_Ptr)
{
	Write_24L512_nByte(ucRDAddress,Counter,Data_Ptr);
	rt_kprintf("write date on address 0x%06x:%s\n",ucRDAddress,Data_Ptr);
}


FINSH_FUNCTION_EXPORT(EEPROMReadTest, EEPROM Read Test.)
FINSH_FUNCTION_EXPORT(EEPROMWriteTest, EEPROM Write Test.)
FINSH_FUNCTION_EXPORT(ReadE, read EEPROM ucRDAddress[U32] Counter[U8].)
FINSH_FUNCTION_EXPORT(WriteE, write EEPROM ucRDAddress[U32] Counter[U8] Data_Ptr[U8 *].)


#endif
#endif
