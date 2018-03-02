#include "mcp1316.h"
#include "rthw.h"

void MCP1316_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MCP1316_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = MCP1316_PIN;
    GPIO_Init(MCP1316_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(MCP1316_GPIO, MCP1316_PIN);
}

//标准踢狗时间为50ns
void MCP1316_kickwatchdog(void)
{
	//先拉低电平   然后再拉高
	GPIO_ResetBits(MCP1316_GPIO, MCP1316_PIN);
	rt_hw_us_delay(1);
	GPIO_SetBits(MCP1316_GPIO, MCP1316_PIN);
	
}
