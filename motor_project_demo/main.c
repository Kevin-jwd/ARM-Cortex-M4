#include "device_driver.h"
#include <stdio.h>
#include <macro.h>

#define BAUD_RATE  (115200U)

#define GPIOA0_PIN      (0U)
#define GPIOA1_PIN      (1U)
#define GPIOC_PIN      (13U)
#define DELAY_CNT (1000000U)

#define OUTPUT_PUSH_PULL            (0x0)
#define GENERAL_PURPOSE_OUTPUT_MODE (0x1)
#define TURN_RIGHT_SIGNAL           (0x1)
#define TURN_LEFT_SIGNAL            (0x0)

#define RXNE_BIT       (5U)

typedef enum {
	STOP,
	TURN_RIGHT,
	TURN_LEFT
} STATE;

static void Sys_Init(int baud)
{
	SCB->CPACR |= (0x3 << 10 * 2) | (0x3 << 11 * 2);
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
}

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

void Main(void)
{
	Sys_Init(BAUD_RATE);
	Key_Poll_Init();
	printf("LED Toggling : Macro\n");
	
	volatile int i;

	// PA0, PA1 : General purpose output mode (0101)
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, (GPIOA0_PIN * 2U));
	
	// PA0, PA1 : Output push-pull (00)
	Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, GPIOA0_PIN);

	// Initialize Motor
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, GPIOA0_PIN);

	int state = STOP;
	volatile int i = 0;

	for(;;)
	{

		
	}
}