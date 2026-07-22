#include "device_driver.h"
#include <stdio.h>

#define BAUD_RATE       (115200U)
#define STOP_MSEC         (3000U)
#define STOP_DELAY_MSEC    (300U)
#define STATE_CNT            (2U)

#define TWO_BIT            (0x3U)
#define FOUR_BIT           (0xFU)

#define PUSH_PULL_MODE      (0x0U)
#define GENERAL_OUTPUT_MODE (0x1U)

#define GPIOA0_PIN           (0U)
#define GPIOA1_PIN           (1U)
#define GPIOC_PIN           (13U)
#define DELAY_CNT      (1000000U)

#define FOUR_BIT_GENERAL_PURPOSE_OUTPUT_MODE       (0x5U)
#define TWO_BIT_OUTPUT_PUSH_PULL_MODE              (0x0U)

#define TURN_RIGHT_SIGNAL (0x2U)
#define TURN_LEFT_SIGNAL  (0x1U)
#define STOP_SIGNAL       (0x0U)

#define RXNE_BIT         (5U)

static void Sys_Init(int baud);
void Turn_Right(void);
void Turn_Left(void);
void Stop(void);

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
	Macro_Write_Block(GPIOA->MODER, FOUR_BIT, FOUR_BIT_GENERAL_PURPOSE_OUTPUT_MODE, (GPIOA0_PIN * 2U));
	
	// PA0, PA1 : Output push-pull (00)
	Macro_Write_Block(GPIOA->OTYPER, TWO_BIT, TWO_BIT_OUTPUT_PUSH_PULL_MODE, GPIOA0_PIN);
	
	// Initialize Motor
	Stop();
	// Macro_Write_Block(GPIOA->ODR, TWO_BIT, STOP_SIGNAL, GPIOA0_PIN);
	
	int state = STOP;
	int prev_state = state;
	int end_flag = 0;
	
	for(;;)
	{
		if(Key_Get_Pressed())
		{ 
			TIM2_Start_OnePulse(STOP_MSEC);
			while(!Macro_Check_Bit_Set(GPIOC->IDR, GPIOC_PIN))
			{
				if(TIM2_Check_Timeout())
				{
					end_flag = 1;
				}
			}
			state = (end_flag) ? STOP : (state == STOP) ? TURN_RIGHT : (state + 1) % STATE_CNT;
			end_flag = 0;
		}
		
		if(state != prev_state)
		{
			Stop();
			TIM2_Delay(STOP_DELAY_MSEC);
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

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

void Turn_Right(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_RIGHT_SIGNAL, GPIOA0_PIN);
}

void Turn_Left(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_LEFT_SIGNAL, GPIOA0_PIN);
}

void Stop(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, STOP_SIGNAL, GPIOA0_PIN);
}