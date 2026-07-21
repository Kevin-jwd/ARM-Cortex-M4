#include "device_driver.h"
#include <stdio.h>

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

void Turn_Right(void);
void Turn_Left(void);
void Stop(void);

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

typedef enum {
	TURN_RIGHT,
	TURN_LEFT,
	STOP
} STATE;

void Main(void)
{
	Sys_Init(BAUD_RATE);
	Key_Poll_Init();
	printf("Motor Start\n");

	// PA0, PA1 : General purpose output mode (0101)
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, (GPIOA0_PIN * 2U));
	
	// PA0, PA1 : Output push-pull (00)
	Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, GPIOA0_PIN);

	// Initialize Motor
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, GPIOA0_PIN);

	int state = STOP;
	int prev_state = state;
	int end_flag = 0;

	for(;;)
	{
		if(Key_Get_Pressed())
		{ 
			TIM2_Start_OnePulse(3*1000);
			while(!Macro_Check_Bit_Set(GPIOC->IDR, 13))
			{
				if(TIM4_Check_Timeout())
				{
					end_flag = 1;
				}
			}
			state = (end_flag) ? STOP : (state + 1) % 2;
			end_flag = 0;
		}

		switch(state)
		{
			case TURN_RIGHT:
				Turn_Right();
				break;
			case TURN_LEFT:
				Turn_Left();
				break;
			default:
				Stop();
				break;
		}
		prev_state = state;
	}
}

void Turn_Right(void)
{
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x2, GPIOA0_PIN);
}

void Turn_Left(void)
{
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x1, GPIOA0_PIN);
}

void Stop(void)
{
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, GPIOA0_PIN);
}