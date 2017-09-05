/*****************************************************************************/
/* File      : rt_stm32f10x_spi.c                                            */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "rt_stm32f10x_spi.h"
#include "rthw.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define W25_RCC                    RCC_APB2Periph_GPIOA
#define CLK_GPIO                   GPIOA
#define CLK_PIN                    (GPIO_Pin_6)

#define DO_GPIO                   GPIOA
#define DO_PIN                    (GPIO_Pin_4)

#define DI_GPIO                   GPIOA
#define DI_PIN                    (GPIO_Pin_5)

#define CS_GPIO                   GPIOA
#define CS_PIN                    (GPIO_Pin_3)

#define IO_SPI1_CLK_H()      GPIO_SetBits(CLK_GPIO, CLK_PIN)   	//CLK设置为高电平
#define IO_SPI1_CLK_L()      GPIO_ResetBits(CLK_GPIO, CLK_PIN);	//CLK设置为低电平
#define IO_SPI1_MOSI_H()     GPIO_SetBits(DI_GPIO, DI_PIN)			//MOSI设置为高电平
#define IO_SPI1_MOSI_L()     GPIO_ResetBits(DI_GPIO, DI_PIN)		//MOSI设置为低电平

// MISO状态获取
#define IO_SPI1_MISO_STATE() GPIO_ReadInputDataBit(DO_GPIO, DO_PIN) 

/*****************************************************************************/
/*                                                                           */
/*  Function Implementations                                                 */
/*                                                                           */
/*****************************************************************************/
static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration);
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message);

static struct rt_spi_ops stm32_spi_ops =
{
    configure,
    xfer
};

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    return RT_EOK;
};

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
//    struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
    struct stm32_spi_cs * stm32_spi_cs = device->parent.user_data;
    rt_uint32_t size = message->length;

    /* take CS */
    if(message->cs_take)
    {
        GPIO_ResetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }

		
		{
    const rt_uint8_t * send_ptr = message->send_buf;
    rt_uint8_t * recv_ptr = message->recv_buf;
    rt_uint8_t i;
    while(size--) {
      rt_uint8_t data = 0xFF;
      rt_uint8_t u8Rtn = 0;
      if(send_ptr != RT_NULL) {
        data = *send_ptr++;
      }

      // 循环移出一个字节数据
      for(i=0; i<8; i++)
      {
        IO_SPI1_CLK_L();
        if((data & 0x80) != 0)  {
          IO_SPI1_MOSI_H();
        } else {
          IO_SPI1_MOSI_L();
        }
        data <<= 1;
        u8Rtn <<= 1;
        if(IO_SPI1_MISO_STATE() != 0)  {
          u8Rtn |= 0x01;
        }
        IO_SPI1_CLK_H();
        
      }
	  
      if(recv_ptr != RT_NULL) {
        *recv_ptr++ = u8Rtn;
      }
    }
  }

    /* release CS */
    if(message->cs_release)
    {
        GPIO_SetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }	
		
    return message->length;
};

/** \brief init and register stm32 spi bus.
 *
 * \param SPI: STM32 SPI, e.g: SPI1,SPI2,SPI3.
 * \param stm32_spi: stm32 spi bus struct.
 * \param spi_bus_name: spi bus name, e.g: "spi1"
 * \return
 *
 */
rt_err_t stm32_spi_register(SPI_TypeDef * SPI,
                            struct stm32_spi_bus * stm32_spi,
                            const char * spi_bus_name)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(W25_RCC,ENABLE);

  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin   = CLK_PIN;
  GPIO_Init(CLK_GPIO, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin   = DI_PIN;
  GPIO_Init(DI_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = CS_PIN;
  GPIO_Init(CS_GPIO, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin   = DO_PIN;
  GPIO_Init(DO_GPIO, &GPIO_InitStructure);
	
	GPIO_ResetBits(CLK_GPIO, CLK_PIN);							                         //set clock to low initial state for SPI operation mode 0
//	GPIO_SetBits(CLK_GPIO, CLK_PIN);							                         //set clock to High initial state for SPI operation mode 3	
	GPIO_SetBits(CS_GPIO, CS_PIN);

  return rt_spi_bus_register(&stm32_spi->parent, spi_bus_name, &stm32_spi_ops);
}
