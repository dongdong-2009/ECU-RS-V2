#ifndef __MCP1316_H__
#define __MCP1316_H__

#include <stm32f10x.h>

#define MCP1316_RCC             	RCC_APB2Periph_GPIOC
#define MCP1316_GPIO        			GPIOC
#define MCP1316_PIN               (GPIO_Pin_3)


void MCP1316_init(void);
void MCP1316_kickwatchdog(void);

#endif /*__MCP1316_H__*/
