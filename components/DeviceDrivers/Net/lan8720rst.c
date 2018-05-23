/*****************************************************************************/
/* File      : lan8720rst.c                                                  */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <rtthread.h>
#include <stm32f10x.h>
#include <rthw.h>
#include "stm32_eth.h"
#include "stdio.h"
#include "powerIO.h"
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define LAN8720_RCC                  	RCC_APB2Periph_GPIOB
#define LAN8720_GPIO              		GPIOB
#define LAN8720_PIN                  	(GPIO_Pin_14)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
void rt_hw_lan8720_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LAN8720_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LAN8720_PIN;
    GPIO_Init(LAN8720_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(LAN8720_GPIO, LAN8720_PIN);
}

void rt_hw_lan8720_rst(void)
{
  GPIO_ResetBits(LAN8720_GPIO, LAN8720_PIN);
	rt_hw_s_delay(1);
	GPIO_SetBits(LAN8720_GPIO, LAN8720_PIN);
}

int rt_hw_GetWiredNetConnect(void)
{
	/*	REV1.0.6¸ü¸ÄÎªIOÅÐ¶Ï 20180510
	int value = 0,ret = 0;
	value = ETH_ReadPHYRegister(0x00, 1);
	rt_hw_ms_delay(5);
	value = ETH_ReadPHYRegister(0x00, 1);
	ret = (value & (1 << 2)) >> 2;
	return ret;
	*/
	return rt_hw_ETHIO_status();
}

#if 1
#ifdef RT_USING_FINSH
#include <finsh.h>
void lanrst()
{
	rt_hw_lan8720_rst();
}
FINSH_FUNCTION_EXPORT(lanrst, lanrst.)
#endif
#endif

