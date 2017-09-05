/*****************************************************************************/
/* File      : rt_stm32f10x_spi.h                                            */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef STM32_SPI_H_INCLUDED
#define STM32_SPI_H_INCLUDED

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <rtdevice.h>
#include "stm32f10x.h"
#include "board.h"

struct stm32_spi_bus
{
    struct rt_spi_bus parent;
    SPI_TypeDef * SPI;
};


struct stm32_spi_cs
{
    GPIO_TypeDef * GPIOx;
    uint16_t GPIO_Pin;
};

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

/* public function list */
rt_err_t stm32_spi_register(SPI_TypeDef * SPI,
                            struct stm32_spi_bus * stm32_spi,
                            const char * spi_bus_name);

#endif // STM32_SPI_H_INCLUDED
