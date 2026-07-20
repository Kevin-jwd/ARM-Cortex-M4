#include "device_driver.h"

#define GPIOC_PIN      (13U)

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_Get_Pressed(void)
{
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
