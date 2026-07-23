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

// .weak   EXTI15_10_IRQHandler
// .thumb_set EXTI15_10_IRQHandler, Invalid_ISR
void EXTI15_10_IRQHandler(void)
{
	// KEY Pressed 메시지 인쇄
	printf("KEY Pressed !!!!!\n");

	// KEY(EXTI) Pending Clears
	EXTI->PR = (PENDING_CLEAR_BIT << PIN_NUM);

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}
