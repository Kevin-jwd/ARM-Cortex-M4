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

/* Key 인식 */

#if 0

#define BAUD_RATE (115200U)
#define GPIOC_PIN     (13U)
#define GPIOA_PIN      (5U)

void Main(void)
{
	int data = 0U;

	Sys_Init(BAUD_RATE);
	printf("KEY Input Test #1\n");

	/* 아래 코드 수정 금지 : Port-C Clock Enable */
	Macro_Set_Bit(RCC->AHB1ENR, 2U); 

	// KEY(PC13)을 GPIO 입력으로 선언
	Macro_Write_Block(GPIOC->MODER, 0x3U, 0x0U, (uint32_t)(GPIOC_PIN*2));
	LED_Off();

	for(;;)
	{
		data = Macro_Extract_Area(~(GPIOC->IDR), 0x1U, GPIOC_PIN);
		Macro_Write_Block(GPIOA->ODR, 0x1U, data, GPIOA_PIN);
	}
}

#endif

/* Key에 의한 LED Toggling_mine */

#if 0

#define BAUD_RATE (115200U)
#define GPIOC_PIN     (13U)
#define GPIOA_PIN      (5U)

void Main(void)
{
	int prev_state = 0U;
	int curr_state;

	Sys_Init(BAUD_RATE);
	printf("KEY Input Toggling #1\n");

	Macro_Set_Bit(RCC->AHB1ENR, 2U); 
	Macro_Write_Block(GPIOC->MODER, 0x3U, 0x0U, (uint32_t)(GPIOC_PIN*2U));

	for(;;)
	{ 
		curr_state = Macro_Extract_Area(GPIOC->IDR, 0x1U, GPIOC_PIN);
		// Falling Edge Detection
		if((prev_state != 0U) && (curr_state == 0U))
		{
			Macro_Invert_Bit(GPIOA->ODR, GPIOA_PIN);
		}
		prev_state = curr_state;
	}
}

#endif

/* Key에 의한 LED Toggling */

#if 0

#define BAUD_RATE (115200U)
#define GPIOC_PIN     (13U)
#define GPIOA_PIN      (5U)

void Main(void)
{
	int prev_state = 0U;
	int curr_state;

	Sys_Init(BAUD_RATE);
	printf("KEY Input Toggling #1\n");

	Macro_Set_Bit(RCC->AHB1ENR, 2U); 
	Macro_Write_Block(GPIOC->MODER, 0x3U, 0x0U, (uint32_t)(GPIOC_PIN*2U));

	for(;;)
	{ 
		if(Macro_Check_Bit_Clear(GPIOC->IDR, GPIOC_PIN)) Macro_Invert_Bit(GPIOA->ODR, GPIOA_PIN);
	}
}

#endif

#if 1

#define BAUD_RATE (115200U)
#define GPIOC_PIN      (7U)
#define GPIOA_PIN      (5U)

void Main(void)
{
	int data;

	Sys_Init(BAUD_RATE);
	printf("KEY Input Toggling #1\n");

	// 1. Enable Port-C Clock 
	Macro_Set_Bit(RCC->AHB1ENR, 2U); 

	// 2. Set Internal Pull-Up (01)
	Macro_Write_Block(GPIOC->PUPDR, 0x3U, 0x1U, (uint32_t)(GPIOC_PIN*2U));

	// 3. Set PC7 input mode (00)
	Macro_Write_Block(GPIOC->MODER, 0x3U, 0x0U, (uint32_t)(GPIOC_PIN*2U));

	for(;;)
	{ 
		data = Macro_Extract_Area(~(GPIOC->IDR), 0x1U, GPIOC_PIN);
		Macro_Write_Block(GPIOA->ODR, 0x1U, data, GPIOA_PIN);
	}
}

#endif