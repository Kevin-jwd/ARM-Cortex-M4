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

#if 1

#define BAUD_RATE (115200U)
#define TXE_BIT        (7U)
#define RXNE_BIT       (5U)

void Main(void)
{
	Sys_Init(BAUD_RATE);
	printf("\nUART Echo-Back Test\n");

	Uart1_Init(BAUD_RATE);

	for(;;)
	{
		// 수신이 있으면 그대로 송신 (echo back)
		char x;
		// SR 레지스터 5번 bit (RXNE)가 Set (1: full) 면 탈출
		while(!Macro_Check_Bit_Set(USART1->SR, RXNE_BIT))
		{
			;
		}
		x = USART1->DR;
		if(x == 'q') 
		{
			printf("\nExit!!!!!\n");
			break;
		}
		// SR 레지스터 7번 bit (TXE)가 Set (1: empty) 면 탈출
		while(!Macro_Check_Bit_Set(USART1->SR, TXE_BIT))
		{
			;
		}
		USART1->DR = x;
	}
}
#endif

#if 0
#define BAUD_RATE 115200U
#define TXE_BIT        7U

void Main(void)
{
	Sys_Init(BAUD_RATE);
	printf("\nUART Echo-Back Test\n");

	Uart1_Init(BAUD_RATE);

	// SR 레지스터 7번 bit (TXE)가 Set (1: empty) 면 탈출
	while(!Macro_Check_Bit_Set(USART1->SR, TXE_BIT))
	{
		;
	}

	// USART1-DR = 'A'
	USART1->DR = 'A';
	while(!Macro_Check_Bit_Set(USART1->SR, TXE_BIT))
	{
		;
	}

	// USART1-DR = 'B'
	USART1->DR = 'B';
	while(!Macro_Check_Bit_Set(USART1->SR, TXE_BIT))
	{
		;
	}

	// USART1-DR = 'C'
	USART1->DR = 'C';
	while(!Macro_Check_Bit_Set(USART1->SR, TXE_BIT))
	{
		;
	}
}
#endif