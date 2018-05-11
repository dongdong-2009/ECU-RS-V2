#include <rtthread.h>
#include <stm32f10x.h>

#define powerio_rcc                    RCC_APB2Periph_GPIOC
#define powerio_gpio                   GPIOC
#define powerio_pin                    (GPIO_Pin_13)

#define ethio_rcc                    RCC_APB2Periph_GPIOA
#define ethio_gpio                   GPIOA
#define ethio_pin                    (GPIO_Pin_6)

void rt_hw_powerIO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(powerio_rcc,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = powerio_pin;
    GPIO_Init(powerio_gpio, &GPIO_InitStructure);
}

void rt_hw_powerIO_on(void)
{
    GPIO_SetBits(powerio_gpio, powerio_pin);
}

void rt_hw_powerIO_off(void)
{
    GPIO_ResetBits(powerio_gpio, powerio_pin);
}


void rt_hw_ETHIO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ethio_rcc,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = ethio_pin;
    GPIO_Init(ethio_gpio, &GPIO_InitStructure);
		

}

int rt_hw_ETHIO_status(void)
{
	return GPIO_ReadInputDataBit(ethio_gpio,ethio_pin);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
void power(rt_uint32_t value)
{
	rt_hw_powerIO_init();

    /* set led status */
    switch (value)
    {
        case 0:
            rt_hw_powerIO_off();
            break;
        case 1:
            rt_hw_powerIO_on();
            break;
        default:
			break;
    }

}
void ethio(void)
{
	rt_hw_ETHIO_init();
	rt_kprintf("ethio:%d\n",rt_hw_ETHIO_status());
}
FINSH_FUNCTION_EXPORT(power, set power on[1] or off[0].)
FINSH_FUNCTION_EXPORT(ethio, ethio.)
#endif
