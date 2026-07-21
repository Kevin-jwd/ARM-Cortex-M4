#include "device_driver.h"

#define GPIOC_PIN      (13U)

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
