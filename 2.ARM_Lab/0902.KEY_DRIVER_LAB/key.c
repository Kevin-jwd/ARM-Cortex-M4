#include "device_driver.h"

#define GPIOC_PIN      (13U)

void Key_Poll_Init(void)
{
	// AHB1ENR
	// AHB1 버스에 연결된 주변장치들의 CLK Enable
	Macro_Set_Bit(RCC->AHB1ENR, 2); 

	// PORTC 13번 Input mode
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_Get_Pressed(void)
{
	// IDR: Input Data Register
	// 입력 속성으로 설정된 핀의 입력 값을 확인
	// Key가 눌렸으면 0, 안 눌렸으면 1 리턴
	return (Macro_Check_Bit_Clear(GPIOC->IDR, GPIOC_PIN));
}

void Key_Wait_Key_Pressed(void)
{
	// Key가 눌리지 않은 동안 (1)
	while(Macro_Check_Bit_Set(GPIOC->IDR, GPIOC_PIN))
	// while(!Macro_Check_Bit_Clear(GPIOC->IDR, GPIOC_PIN))
	{
		;
	}
	// return;
}

void Key_Wait_Key_Released(void)
{
	// Key가 눌린 동안 (0)
	while(Macro_Check_Bit_Clear(GPIOC->IDR, GPIOC_PIN))
	// while(!Macro_Check_Bit_Set(GPIOC->IDR, GPIOC_PIN))
	{
		;
	}
	// return;
}
