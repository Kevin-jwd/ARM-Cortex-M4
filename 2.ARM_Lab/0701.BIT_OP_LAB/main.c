#include "device_driver.h"
#include <stdio.h>
#include <macro.h>

static void Sys_Init(int baud)
{
	SCB->CPACR |= (0x3 << 10 * 2) | (0x3 << 11 * 2);
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
}

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED ON : Bit Operation - 1\n");

	GPIOA->MODER  &= ~(0x1 << 11);
	GPIOA->MODER  |=  (0x1 << 10);
	
	GPIOA->OTYPER &= ~(0x1 <<  5);
	
	GPIOA->ODR    |=  (0x1 <<  5);
}

#endif

#if 0

void Main(void)
{
	Sys_Init(115200);
	printf("LED ON : Bit Operation - 1\n");

	/* 비트 연산을 이용하여 LED를 ON하는 코드를 설계하시오 */


}

#endif

#if 1

#define GPIOA_PIN 5U
#define DELAY_CNT 0x80000U
#define BAUD_RATE 115200U

void Main(void)
{
	Sys_Init(BAUD_RATE);
	printf("LED Toggling : Macro\n");

	volatile int i;

	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, (GPIOA_PIN * 2U));
	Macro_Clear_Bit(GPIOA->OTYPER, GPIOA_PIN);

	for (;;)
	{
		Macro_Invert_Bit(GPIOA->ODR, GPIOA_PIN);
		for (i = 0; i < DELAY_CNT; i++)
			;
	}
}

#endif
