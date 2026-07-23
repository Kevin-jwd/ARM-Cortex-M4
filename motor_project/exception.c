#include "device_driver.h"
#include <stdio.h>

// 등록하지 않은 함수 호출
void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

/* Falling edge / Rising edge */
extern volatile uint8_t is_timer_started;
extern volatile uint8_t is_timer_timeout;
extern volatile int is_falling_edge_detected;

void EXTI15_10_IRQHandler(void)
{
	is_falling_edge_detected = 1U-is_falling_edge_detected;//U - is_falling_edge_detected;

	// KEY(EXTI) Pending Clears
	EXTI->PR = (PENDING_CLEAR_BIT << PORTC_SW_PIN);

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}

/* USART2 ISR */
extern volatile uint8_t Uart_Data_In;
extern volatile unsigned char Uart_Data;

void USART2_IRQHandler(void)
{
	// 수신된 데이터는 Uart_Data에 저장
	Uart_Data = USART2->DR;

	// Uart_Data_In Flag Setting
	Uart_Data_In = 1U;

	// SubSource Auto Pending Clear

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(USART2_IRQn);
}

/* Timer2 ISR (for Stop) */
void TIM2_IRQHandler(void)
{
	is_timer_timeout = 1U;

	// TIM4 Interrupt Pending Clear
	Macro_Clear_Bit(TIM2->SR, UIF_BIT);

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);
}

extern volatile uint8_t is_stop_delay_timeout;

/* Timer4 ISR (for Short Stop) */
void TIM4_IRQHandler(void)
{
	is_stop_delay_timeout = 1U;

	// TIM4 Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR, UIF_BIT);

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(TIM4_IRQn);
}