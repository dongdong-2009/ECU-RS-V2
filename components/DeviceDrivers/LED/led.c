/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>

#define led1_rcc                    RCC_APB2Periph_GPIOC
#define led1_gpio                   GPIOC
#define led1_pin                    (GPIO_Pin_9)

void rt_hw_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(led1_rcc,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = led1_pin;
    GPIO_Init(led1_gpio, &GPIO_InitStructure);
}

void rt_hw_led_on(void)
{
    GPIO_ResetBits(led1_gpio, led1_pin);
}

void rt_hw_led_off(void)
{
    GPIO_SetBits(led1_gpio, led1_pin);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_uint8_t led_inited = 0;
void led(rt_uint32_t value)
{
    /* init led configuration if it's not inited. */
    if (!led_inited)
    {
        rt_hw_led_init();
        led_inited = 1;
    }
    /* set led status */
    switch (value)
    {
        case 0:
            rt_hw_led_off();
            break;
        case 1:
            rt_hw_led_on();
            break;
        default:
			break;
    }

}
FINSH_FUNCTION_EXPORT(led, set led on[1] or off[0].)
#endif

