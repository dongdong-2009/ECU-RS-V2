#include "var.h"
#include "CMT2300.h"
#include "string.h"
#include "rthw.h"
#include "SEGGER_RTT.h"
#include "rtthread.h"


#ifdef RFM_433
byte cmt2300A_para[FTP8_LENGTH]=
{
//[CMT Bank]

0x00,  //0x00
0x66,  //0x01
0xEC,  //0x02
0x1D,  //0x03
0xF0,  //0x04
0x80,  //0x05
0x14,  //0x06
0x08,  //0x07
0x91,  //0x08
0x03,  //0x09
0x02,  //0x0A
0xD0,  //0x0B

//[System Bank]

0xAE,  //0x0C
0xE0,  //0x0D
0x35,  //0x0E
0x00,  //0x0F
0x00,  //0x10
0xF4,  //0x11
0x10,  //0x12
0xE2,  //0x13
0x42,  //0x14
0x20,  //0x15
0x00,  //0x16
0x81,  //0x17

//[Frequency Bank]

0x42,  //0x18
0x71,  //0x19
0xCE,  //0x1A
0x1C,  //0x1B
0x42,  //0x1C
0x5B,  //0x1D
0x1C,  //0x1E
0x1C,  //0x1F

//[Data Rate Bank]

0xD3,  //0x20
0x64,  //0x21
0x10,  //0x22
0x33,  //0x23
0xD1,  //0x24
0x35,  //0x25
0x0C,  //0x26
0x0A,  //0x27
0x9F,  //0x28
0x4B,  //0x29
0x29,  //0x2A
0x29,  //0x2B
0xC0,  //0x2C
0x28,  //0x2D
0x0A,  //0x2E
0x53,  //0x2F
0x08,  //0x30
0x00,  //0x31
0xB4,  //0x32
0x00,  //0x33
0x00,  //0x34
0x01,  //0x35
0x00,  //0x36
0x00,  //0x37

//[Baseband Bank]

0x12,  //0x38
0x04,  //0x39
0x00,  //0x3A
0xAA,  //0x3B
0x02,  //0x3C
0x00,  //0x3D
0x00,  //0x3E
0x00,  //0x3F
0x00,  //0x40
0x00,  //0x41
0x00,  //0x42
0xD4,  //0x43
0x2D,  //0x44
0x01,  //0x45
0x1F,  //0x46
0x00,  //0x47
0x00,  //0x48
0x00,  //0x49
0x00,  //0x4A
0x00,  //0x4B
0x01,  //0x4C
0x00,  //0x4D
0x00,  //0x4E
0x60,  //0x4F
0xFF,  //0x50
0x00,  //0x51
0x00,  //0x52
0x1F,  //0x53
0x10,  //0x54

//[TX Bank]

0x50,  //0x55
0x4D,  //0x56
0x06,  //0x57
0x00,  //0x58
0x0F,  //0x59
0x70,  //0x5A
0x00,  //0x5B
0x8A,  //0x5C
0x18,  //0x5D
0x3F,  //0x5E
0x7F   //0x5F
};
#endif

#ifdef RFM_915
unsigned char cmt2300A_para[FTP8_LENGTH]=
{
//[CMT Bank]
//Addr  Value
0x00,//0x00  
0x66,//0x01  
0xEC,//0x02  
0x1D,//0x03  
0xF0,//0x04  
0x80,//0x05  
0x14,//0x06
0x08,//0x07  
0x11,//0x08  
0x03,//0x09  
0x02,//0x0A  
0x00,//0x0B  

//[System Bank]
//Addr  Value
0xAE,//0x0C  
0xE0,//0x0D  
0x35,//0x0E  
0x00,//0x0F  
0x00,//0x10  
0xF4,//0x11  
0x10,//0x12  
0xE2,//0x13  
0x42,//0x14  
0x20,//0x15  
0x00,//0x16  
0x81,//0x17  

//[Frequency Bank]
//Addr  Value
0x46,//0x18  
0x4C,//0x19  
0xA2,//0x1A  
0x87,//0x1B  
0x46,//0x1C  
0x41,//0x1D  
0x49,//0x1E  
0x17,//0x1F  

//[Data Rate Bank]
//Addr  Value
0x32,//0x20  
0x18,//0x21  
0x10,//0x22  
0x99,//0x23  
0xC2,//0x24  
0xBD,//0x25  
0x06,//0x26  
0x0B,//0x27  
0x9F,//0x28  
0x26,//0x29  
0x29,//0x2A  
0x29,//0x2B  
0xC0,//0x2C  
0x51,//0x2D  
0x2A,//0x2E  
0x53,//0x2F  
0x00,//0x30  
0x00,//0x31  
0xB4,//0x32  
0x00,//0x33  
0x00,//0x34  
0x01,//0x35  
0x00,//0x36  
0x00,//0x37  

//[Baseband Bank]
//Addr  Value
0x12,//0x38  
0x04,//0x39  
0x00,//0x3A  
0xAA,//0x3B  
0x02,//0x3C  
0x00,//0x3D  
0x00,//0x3E  
0x00,//0x3F  
0x00,//0x40  
0x00,//0x41  
0x00,//0x42  
0xD4,//0x43  
0x2D,//0x44  
0x01,//0x45  
0x1F,//0x46  
0x00,//0x47  
0x00,//0x48  
0x00,//0x49  
0x00,//0x4A  
0x00,//0x4B  
0x01,//0x4C  
0x00,//0x4D  
0x00,//0x4E  
0x60,//0x4F  
0xFF,//0x50  
0x00,//0x51  
0x00,//0x52  
0x3F,//0x53  
0x10,//0x54  

//[TX Bank]
//Addr  Value
0x50,//0x55  
0x26,//0x56  
0x03,//0x57  
0x00,//0x58  
0x42,//0x59  
0xB0,//0x5A  
0x00,//0x5B  
0x8A,//0x5C  
0x18,//0x5D  
0x3F,//0x5E  
0x7F //0x5F  
};

#endif

void InputSDA(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RFM300M_SDIO_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SDIO_PIN;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;      //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz
	GPIO_Init(SDIO_GPIO, &GPIO_InitStructure);
}
void	OutputSDA(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RFM300M_SDIO_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SDIO_PIN;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz
	GPIO_Init(SDIO_GPIO, &GPIO_InitStructure);
}

// spi Init
void Spi3Init(void)
{	
	//将芯片上的引脚都配置为推挽输出 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RFM300M_SDIO_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SDIO_PIN;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    //最高输出速率50MHz
	GPIO_Init(SDIO_GPIO, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RFM300M_CSB_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = CSB_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    //最高输出速率50MHz	
	GPIO_Init(CSB_GPIO, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RFM300M_SCK_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    //最高输出速率50MHz	
	GPIO_Init(SCK_GPIO, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RFM300M_FCSB_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = FCSB_PIN;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    //最高输出速率50MHz  
	GPIO_Init(FCSB_GPIO, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RFM300M_GPIO3_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO3_PIN; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //最高输出速率50MHz 	       
	GPIO_Init(GPIO3_GPIO, &GPIO_InitStructure);
	
	SetCSB();
	SetFCSB();
	SetSDA();
	ClrSCL();
	// _CSB = 1;
  //_FCSB = 1;
  //_SDA = 1;
	//_SCL = 0;
}
// spi read register
byte Spi3ReadReg(byte addr)
{
	byte i,val;
	val=0x00;
	addr|=0x80;

   // Set CSB to low level and delay
	Spi3Init();
	ClrCSB();
	rt_hw_us_delay(SPI3_SPEED);
	
	//write address
	for (i=0;i<8;i++)
	{
		if (addr&0x80)
			SetSDA();
		else
			ClrSDA();

		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);
		
		addr<<=1;
	}
	SetSDA();
	//read value
	InputSDA();
	for (i=8;i>0;i--)
	{
		val<<=1;
		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);		
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);

		if (SDA_H())
		val|=0x01;
		
	}
    
	// Set CLK to low level and delay
	ClrSCL();
	rt_hw_us_delay(SPI3_SPEED);
	Spi3Init();

	// return value
	return val;
}

// spi write register
void Spi3WriteReg(byte addr,byte value)
{
	byte i;
    
	addr&=~0x80;

    // Set CLK to low level and delay
	Spi3Init();
	ClrCSB();
	rt_hw_us_delay(SPI3_SPEED);
	
	//write address
	for (i=0;i<8;i++)
	{
		if (addr&0x80)
			SetSDA();
		else
			ClrSDA();

		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);
		
		addr<<=1;
	}

	//write value	
	for (i=0;i<8;i++)
	{
		if (value&0x80)
			SetSDA();
		else
			ClrSDA();

		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);

		value<<=1;
	}
    
	// Set CLK to low level and delay
	ClrSCL();
	rt_hw_us_delay(SPI3_SPEED);
	Spi3Init();
}

// spi read one byte from the fifo
byte Spi3ReadFIFOByte(void)
{
	byte i,val;
	val=0x00;
	//set FCSB to low and delay
	Spi3Init();
	ClrFCSB();
	InputSDA();
	rt_hw_us_delay(SPI3_SPEED<<2);
    // read one byte
    for (i=0;i<8;i++)
    {
		val<<=1;
		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);
		
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);
		
		if (SDA_H())
		val|=0x01;

    }
    // set SCL to low and delay more than 2us
    ClrSCL();
	rt_hw_us_delay(SPI3_SPEED<<1);
	// set FCSB to high and delay more than 4us
	Spi3Init();
	rt_hw_us_delay(SPI3_SPEED<<2);
	
	// return value
	return val;

}
// SPI write one byte to the fifo
void Spi3WriteFIFOByte(byte value)
{
	byte i;
	//set FCSB to low and delay
	Spi3Init();
	ClrFCSB();
	rt_hw_us_delay(SPI3_SPEED<<2);
	
    // write one byte
    for (i=0;i<8;i++)
    {
		
		if (value&0x80)
			SetSDA();
		else
			ClrSDA();
		
		
		ClrSCL();
		rt_hw_us_delay(SPI3_SPEED);
		SetSCL();
		rt_hw_us_delay(SPI3_SPEED);

		value<<=1;

    }
    // set SCL to low and delay more than 2us
    ClrSCL();
	rt_hw_us_delay(SPI3_SPEED<<1);
	// set FCSB to high and delay more than 4us
	Spi3Init();
	rt_hw_us_delay(SPI3_SPEED<<2);

}

// set configuration  bank of the register 
void SetConfigBank(void)
{
	byte i;
 #ifdef  RFM_433
    if(RFM300H_Rate<4)
    {
        //if(RFM300H_Rate==0)
        //{
        //    memcpy(&cmt2300A_para[32], RFM300H_Rate_2K,17);
        //    memcpy(&cmt2300A_para[89], &RFM300H_Rate_2K[17],2);
        //}
        if(RFM300H_Rate==1)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_5K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_5K[17],2);
        }
        if(RFM300H_Rate==2)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_10K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_10K[17],2);
        }
        if(RFM300H_Rate==3)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_20K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_20K[17],2);
        }
    }
	for (i=0;i<FTP8_LENGTH;i++)
		Spi3WriteReg(i,cmt2300A_para[i]);	
#endif


#ifdef RFM_915
		if(RFM300H_Rate<5)
    {
        if(RFM300H_Rate==1)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_2K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_2K[17],2);
        }
        if(RFM300H_Rate==2)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_5K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_5K[17],2);
        }
        if(RFM300H_Rate==3)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_10K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_10K[17],2);
        }
        if(RFM300H_Rate==4)
        {
            memcpy(&cmt2300A_para[32], RFM300H_Rate_20K,17);
            memcpy(&cmt2300A_para[89], &RFM300H_Rate_20K[17],2);
        }       
    }
		for (i=0;i<FTP8_LENGTH;i++)
			Spi3WriteReg(i,cmt2300A_para[i]);	
#endif
}

// read the RSSI value
byte ReadRssiValue(byte unit_dbm)
{
	if (unit_dbm)
		return(128-Spi3ReadReg(CMT23_RSSI_DBM));
	else
		return (Spi3ReadReg(CMT23_RSSI_CODE));
}

// switch the operation states.
void SetOperaStatus(byte nOperaStatus)
{ 
	Spi3WriteReg(CMT23_MODE_CTL,nOperaStatus);
}


// get the operation states.
byte GetOperaStatus(void)
{
	byte dat; 
	dat=Spi3ReadReg(CMT23_MODE_STA);
	dat&=MODE_MASK_STA;
	return dat;
}

//Soft reset
void SoftReset(void)
{ 
	Spi3WriteReg(CMT23_SOFTRST,0xFF);
	// at least 10ms
	rt_hw_ms_delay(20); 
}
// disable EEPROM configuration 
// 1 : disable EEPROM configuration ; 0 :enable EEPROM configuration
// void DisableEepromConfig(byte dat)
// {
// 	if (dat)
// 		Spi3WriteReg(REG_EE_DIS,0x04);
// 	else
// 		Spi3WriteReg(REG_EE_DIS,0x00);
// }

// fifo setting
// enable write fifo
void Enable_fifo_write(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CTL);
	val|=SPI_FIFO_RD_WR_SEL;
	Spi3WriteReg(CMT23_FIFO_CTL,val);
}
// enable read fifo
void Enable_fifo_read(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CTL);
	val&=~SPI_FIFO_RD_WR_SEL;
	Spi3WriteReg(CMT23_FIFO_CTL,val);
}
// enable clear fifo
void ClearFifo(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CLR);
	Spi3WriteReg(CMT23_FIFO_FLG,val|FIFO_CLR_RX|FIFO_CLR_TX);
}
// restore fifo
void Enable_fifo_restore(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CLR);
	Spi3WriteReg(CMT23_FIFO_CLR,val|FIFO_RESTORE);
}
void Enable_fifo_TX(void)
{    
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CTL);
	val|=FIFO_RX_TX_SEL;
	Spi3WriteReg(CMT23_FIFO_CTL,val);
}
void Enable_fifo_RX(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CTL);
	val&=~FIFO_RX_TX_SEL;
	Spi3WriteReg(CMT23_FIFO_CTL,val);
}
// merge fifo
void Enable_fifo_merge(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_FIFO_CTL);
	Spi3WriteReg(CMT23_FIFO_CTL,val|FIFO_SHARE_EN);
}

// Set GPIO
void SetGPIO(byte val)
{
	Spi3WriteReg(CMT23_IO_SEL,val);
}
// set interrupt polarity
// level=1 :interrupt is high level, level=0 :interrupt is low level, 
void SetIntPolarity(byte level)
{
	byte val;
	val=Spi3ReadReg(CMT23_INT1_CTL);
	if (level)
	{
		val&=~INT_POLAR;
	}
	else
	{
		val|=INT_POLAR;
	}

	Spi3WriteReg(CMT23_INT1_CTL,val);
}
// select interrupt source 
void SelectIntSource(byte int1,byte int2)
{
	byte tmp;
	tmp = INT_MASK & Spi3ReadReg(CMT23_INT1_CTL);
	Spi3WriteReg(CMT23_INT1_CTL,tmp|int1);
	
	tmp = INT_MASK & Spi3ReadReg(CMT23_INT2_CTL);
	Spi3WriteReg(CMT23_INT2_CTL,tmp|int2);
}

// Enable interrupt
void EnableInt(byte val)
{
	Spi3WriteReg(CMT23_INT_EN,val);
}

// Clear Interrupt
void ClearInt(byte val)
{
	Spi3WriteReg(CMT23_INT_CLR1,0x07);
	Spi3WriteReg(CMT23_INT_CLR2,0x3F);
}

// Get IRQ flag
byte GetIrqFlag_Rx(void)
{
	return(Spi3ReadReg(CMT23_INT_FLG));
}
byte GetIrqFlag_Tx(void)
{
	return(Spi3ReadReg(CMT23_INT_CLR1));
}
// set TX payload length
void SetTxpayloadLength(word len)
{
	byte tmp;
	// enter standby mode
	SetOperaStatus(MODE_GO_STBY);

	tmp = Spi3ReadReg(CMT23_PKT_CTRL1);
	tmp&= 0x8F;
	
// 	if(length!=0)
// 	{
// 		if(FixedPktLength)
// 			len = length-1;
// 		else
// 			len = length;
// 	}
// 	else
// 		len = 0;
	
	tmp|= (((byte)(len>>8)&0x07)<<4);
	Spi3WriteReg(CMT23_PKT_CTRL1,tmp);
	Spi3WriteReg(CMT23_PKT_LEN,(byte)len);

	// enter standby mode
	SetOperaStatus(MODE_GO_SLEEP);

}
// Antenna Diversity to TX
void AntennaDiversity_Tx(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_INT1_CTL);
	val&=0x3F;
	Spi3WriteReg(CMT23_INT1_CTL,RF_SWT1_EN|val);
}
// Antenna Diversity to Rx
void AntennaDiversity_Rx(void)
{
	byte val;
	val=Spi3ReadReg(CMT23_INT1_CTL);
	val&=0x3F;
	Spi3WriteReg(CMT23_INT1_CTL,RF_SWT2_EN|val);
}

// cmt2300 init
void CMT2300_init(void)
{
	// init spi and UART
	Spi3Init();
	//uart.vUartInit(9600, _8N1);
	
	// software reset and enter sleep mode
	SoftReset();
	
    // enter standby mode
	SetOperaStatus(MODE_GO_STBY);
	
	// configuration 
	SetConfigBank();
	
	ClearInt(0x00);
	ClearFifo();
	Enable_fifo_merge();
	// enable TX_DONE ,PACKET_OK interrupt
    EnableInt(TX_DONE_EN|CRC_PASS_EN);
    //
    SelectIntSource(INT_TX_DONE,INT_CRC_PASS);
    
    Spi3WriteReg(0x65,0x22);
		
    //
    //AntennaDiversity_Rx();
	SetOperaStatus(MODE_GO_SLEEP);	
	// go sleep
	SetOperaStatus(MODE_GO_SLEEP);
}

// transmit 
byte SendMessage(byte *p,byte len)
{
	byte i=0;
	byte val=0x00;
	Enable_fifo_TX();
	SetTxpayloadLength(len);

	Enable_fifo_write();
	ClearFifo();
	SetOperaStatus(MODE_GO_STBY);	//MODE_STA_STBY
	// verify transmit state
	do {
		val=GetOperaStatus();
		if (val==MODE_STA_STBY)		//MODE_STA_STBY
			break;
	} while(1);
    // write FIFO
	
	for (i=0;i<len;i++)
	{
		Spi3WriteFIFOByte(p[i]);
	}
	//go transmit

	SetOperaStatus(MODE_GO_TX);
  //rt_hw_ms_delay(300);
    
// verify transmit done
	
	do {
		//Spi3WriteReg(0X68,0X04);
		//rt_hw_ms_delay(500);
		//val=Spi3ReadReg(0X68);
		//SEGGER_RTT_printf(0, "0X68:%x\n",val);
		
		val=GetIrqFlag_Tx();
		//SEGGER_RTT_printf(0, "GetIrqFlag_Tx:%d\n",val);
		if (val==TX_DONE_FLAG)
		{	
			
			ClearInt(0x00);
			break;
		}
		
	} while(1);
	
	// go sleep mode
	SetOperaStatus(MODE_GO_SLEEP);
	RFM300H_SW = 0;	
	return 0x01;
}
// receive
byte GetMessage(byte *p)
{
	int val = 0;
	int index = 0;
	byte i=0x00;
    
	if(RFM300H_SW==0)
	{
		SetOperaStatus(MODE_GO_RX);//进入接收模式
		Enable_fifo_RX();
    Enable_fifo_read();
		do {
			val=GetOperaStatus();
			
			if (val==MODE_STA_RX)
			{
				break;
			}
		} while(1);

			RFM300H_SW = 1; 	
	}
	if(RFM300H_SW==1)
	{	
		for(index =0 ;index<40;index++)
		{
			if(GPIO3==1)
			{
				RFM300H_SW = 2;
				//SEGGER_RTT_printf(0, "GetIrqFlag_Tx:%d\n",index);
				break;
			}
			rt_thread_delay(RT_TICK_PER_SECOND/100);	
		}
		
	}
	if(RFM300H_SW==2)
	{
		unsigned char length = 0;
        length = Spi3ReadFIFOByte();
		for (i=0;i<length;i++)
		{
			p[i]=Spi3ReadFIFOByte();
		}
        ReadRssiValue(0);
        ClearInt(0x00);
		RFM300H_SW = 3;
		return length;
	}
	return 0;
}


char setChannel(char channel)
{
	byte val=0x00;
#ifdef RFM_433	
	if(channel<17)
	{
		if(channel==1)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_433M,8);
		}
		if(channel==2)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_434M,8);
		}
		if(channel==3)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_435M,8);
		}
		if(channel==4)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_436M,8);
		}  
		if(channel==5)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_437M,8);
		}
		if(channel==6)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_438M,8);
		}
		if(channel==7)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_439M,8);
		}
		if(channel==8)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_440M,8);
		} 
		if(channel==9)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_441M,8);
		}
		if(channel==10)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_442M,8);
		}
		if(channel==11)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_443M,8);
		}
		if(channel==12)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_444M,8);
		} 
		if(channel==13)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_445M,8);
		}
		if(channel==14)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_446M,8);
		}
		if(channel==15)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_447M,8);
		}
		if(channel==16)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_448M,8);
		} 
		SetOperaStatus(MODE_GO_STBY);
		do {
			val=GetOperaStatus();
			if (val==MODE_STA_STBY)		//MODE_STA_STBY
				break;
		} while(1);
		SetConfigBank();
		SetOperaStatus(MODE_GO_SLEEP);
		do {
			val=GetOperaStatus();
			if (val==MODE_STA_SLEEP)		//MODE_STA_SLEEP
				break;
		} while(1);
		SEGGER_RTT_printf(0, "setChannel:%d\n",channel);
		return 0;
  } 
	else
	{
		return 1;
	}
#endif
	
#ifdef RFM_915
		if(channel<17)
	{
		if(channel==1)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_915M,8);
		}
		if(channel==2)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_916M,8);
		}
		if(channel==3)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_917M,8);
		}
		if(channel==4)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_918M,8);
		}  
		if(channel==5)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_919M,8);
		}
		if(channel==6)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_920M,8);
		}
		if(channel==7)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_921M,8);
		}
		if(channel==8)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_922M,8);
		} 
		if(channel==9)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_923M,8);
		}
		if(channel==10)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_924M,8);
		}
		if(channel==11)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_925M,8);
		}
		if(channel==12)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_926M,8);
		} 
		if(channel==13)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_927M,8);
		}
		if(channel==14)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_928M,8);
		}
		if(channel==15)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_929M,8);
		}
		if(channel==16)
		{
			memcpy(&cmt2300A_para[24], RFM300H_Channl_930M,8);
		} 
		SetOperaStatus(MODE_GO_STBY);
		do {
			val=GetOperaStatus();
			if (val==MODE_STA_STBY)		//MODE_STA_STBY
				break;
		} while(1);
		SetConfigBank();
		SetOperaStatus(MODE_GO_SLEEP);
		do {
			val=GetOperaStatus();
			if (val==MODE_STA_SLEEP)		//MODE_STA_SLEEP
				break;
		} while(1);
		SEGGER_RTT_printf(0, "setChannel:%d\n",channel);
		return 0;
  } 
	else
	{
		return 1;
	}
#endif
}
	
