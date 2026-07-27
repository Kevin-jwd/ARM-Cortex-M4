#include "device_driver.h"

#define GPIOA_PIN 5U

void LED_Init(void)
{
	/* 아래 코드 수정 금지 : Port-A Clock Enable */
	Macro_Set_Bit(RCC->AHB1ENR, 0); 

	/* MODER: GPIO port mode register */
	// 2비트씩 묶여서 각 포트의 개별 PIN 속성을 결정
	// 00: Input(reset state)
	// 01: General purpose output mode
	// 10: Altrenate function mode
	// ex. PORTA5 -> MODER5[1:0] 11, 10 비트
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, (uint32_t)(GPIOA_PIN * 2));
	
	/* OTYPER: GPIO port output type register */
	// 출력으로 설정된 각 포트의 개별 PIN 출력 타입 결정
	// 0: Output push-pull
	// 1: Output open-drain (0또는 Floating이 출력)
	Macro_Clear_Bit(GPIOA->OTYPER, GPIOA_PIN);

	/* ODR: Output Data Register */
	// 출력 속성으로 설정된 핀의 출력값을 설정
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

