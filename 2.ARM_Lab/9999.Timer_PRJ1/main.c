#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

#if 0
void Main(void)
{
	Sys_Init(115200);
	
	printf("1sec LED toggle Test!!\n");
	
	for(;;)
	{
		LED_On();
		printf("LED_ON!!!!!\n");
		TIM2_Delay(500);
		
		LED_Off();
		printf("LED_OFF!!!!!\n");
		TIM2_Delay(500);
	}
}
#endif

#if 1
void Main(void)
{
	Sys_Init(115200);

	printf("1sec LED toggle Test!!\n");

	TIM4_Repeat(500);

	for(;;)
	{
		if(TIM4_Check_Timeout)
		{
			Macro_Invert_Bit(GPIOA->ODR, 5);
		}
	}
}
#endif