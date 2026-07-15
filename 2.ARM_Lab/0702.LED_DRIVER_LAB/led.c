#include "device_driver.h"

#define GPIOA_PIN 5U

void LED_Init(void)
{
	/* 아래 코드 수정 금지 : Port-A Clock Enable */
	Macro_Set_Bit(RCC->AHB1ENR, 0); 

	// LED를 출력으로 설정하고
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, (uint32_t)(GPIOA_PIN * 2));
	// 초기 Off
	Macro_Clear_Bit(GPIOA->OTYPER, GPIOA_PIN);
	Macro_Clear_Bit(GPIOA->ODR, GPIOA_PIN);
}

void LED_On(void)
{
	Macro_Set_Bit(GPIOA->ODR, GPIOA_PIN);
}

void LED_Off(void)
{
	Macro_Clear_Bit(GPIOA->ODR, GPIOA_PIN);
}

