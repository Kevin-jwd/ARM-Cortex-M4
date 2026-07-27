#include "device_driver.h"

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_Get_Pressed(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, PIN_NUM);	
}

void Key_Wait_Key_Pressed(void)
{
	while(!Macro_Check_Bit_Clear(GPIOC->IDR, PIN_NUM));
}

void Key_Wait_Key_Released(void)
{
	while(!Macro_Check_Bit_Set(GPIOC->IDR, PIN_NUM));
}

void Key_ISR_Enable(int en)
{
	if(en)
	{
		// C Port Clock on
		Macro_Set_Bit(RCC->AHB1ENR, 2); 

		// MODER -> 00: Input mode
		Macro_Write_Block(GPIOC->MODER, TWO_BIT, 0x0, PIN_NUM * 2U);

		// SYSCFG Clock on
		Macro_Set_Bit(RCC->APB2ENR, SYSCFG_CLK_ENABLE_BIT);

		// PC13을 EXTI 13의 소스가 되도록 설정
		// EXTICR4 레지스터: EXTICR[3] -> EXTI[15:12] 설정
		// 4비트씩 묶여있음: FOUR_BIT
		// 0010: C포트
		Macro_Write_Block(SYSCFG->EXTICR[3], FOUR_BIT, EXTICR_PORT_C_BIT, 4U);

		// FTSR: falling trigger selection
		// EXTI 13을 Falling Edge Trigger로 설정
		Macro_Write_Block(EXTI->FTSR, ONE_BIT, FALLING_EDGE_SELECT, PIN_NUM);

		// pending clear는 원인부터 제거해야 하므로 1)EXTI->2)NVIC 순서로 pending clear

		// 1) EXTI 13 Pending Clear - 0: meaningless, 1: pending clear -> 무조건 대입 연산자 사용
		EXTI->PR = (PENDING_CLEAR_BIT << PIN_NUM);
		// Macro_Set_Bit(EXTI->PR, 13U); -> 비트 연산자 사용 X

		// 2) NVIC EXTI15_9 Interrupt Pending Clear
		NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

		// EXTI 13 Interrupt Enable - 0: mask, 1: unmask
		Macro_Set_Bit(EXTI->IMR, PIN_NUM);

		// NVIC EXTI15_9 Interrupt Enable
		NVIC_EnableIRQ(EXTI15_10_IRQn);
	}

	else
	{
		NVIC_DisableIRQ((IRQn_Type)40);
	}
}
