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

volatile int Key_Pressed = 0;
volatile int Uart_Data_In = 0;
volatile unsigned char Uart_Data = 0;

volatile char *p = "ABCDEFGHIJKLMNOPQRSTUV\n";
volatile int Uart_Tx_End = 0U;

void Uart2_TX_Interrupt_Enable(int en) 
{
	if(en)
	{
			// USART2 RX Interrupt Enable
		Macro_Set_Bit(USART2->CR1, TXEIE_BIT);
			// NIVC Pending Clear
		NVIC_ClearPendingIRQ(USART2_IRQn);
			// NVIC Interrupt Enable
		NVIC_EnableIRQ(USART2_IRQn);
	}
	else
	{
		Macro_Clear_Bit(USART2->CR1, TXEIE_BIT);
		NVIC_DisableIRQ(USART2_IRQn);
	}
}

void Main(void)
{
	Sys_Init(115200);
	printf("\nUART2 RX Interrupt Test\n");

	Key_ISR_Enable(1);
	Uart2_TX_Interrupt_Enable(1);
	// Uart2_RX_Interrupt_Enable(1);

	for(;;)
	{
		if(Uart_Tx_End)
		{
			printf("Uart Tx Finished !!!!!\n");
			Uart_Tx_End = 0;
		}

		if(Key_Pressed)
		{
			printf("KEY Pressed!!!\n");
			Key_Pressed = 0;
		}

		if(Uart_Data_In)
		{
			printf("RX Data = %c\n", Uart_Data);
			Uart_Data_In = 0;
		}		

		LED_On();
		TIM2_Delay(500);
		LED_Off();
		TIM2_Delay(500);
	}
}
