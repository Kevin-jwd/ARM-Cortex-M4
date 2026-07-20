#include "device_driver.h"
#include <stdio.h>
#include <macro.h>

#define GPIOA0_PIN 0U
#define GPIOA1_PIN 1U
#define BAUD_RATE 115200U

static void Sys_Init(int baud)
{
	SCB->CPACR |= (0x3 << 10 * 2) | (0x3 << 11 * 2);
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
}

void Main(void)
{
	Sys_Init(BAUD_RATE);
	printf("LED Toggling : Macro\n");

	volatile int i;

	// PA0, PA1 : General purpose output mode (0101)
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, (GPIOA0_PIN * 2U));
	
	// PA0, PA1 : Output push-pull (00)
	Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, GPIOA0_PIN);

	// Turn right (1A: L, 2A: H) (01)
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x1, GPIOA0_PIN);
	for(i=0;i<800000;i++)
	{
		;
	}

	// Fast motor stop
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x3, GPIOA0_PIN);
	for(i=0;i<800000;i++)
	{
		;
	}

	// Turn left (1A: H, 2A: L) (10)
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x2, GPIOA0_PIN);
	for(i=0;i<800000;i++)
	{
		;
	}

	// Fast motor stop
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x3, GPIOA0_PIN);
	for(i=0;i<800000;i++)
	{
		;
	}
}