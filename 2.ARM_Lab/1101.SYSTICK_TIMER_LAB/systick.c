#include "device_driver.h"

#define ENABLE_BIT        (0U)
#define TICKINT_BIT       (1U)
#define CLKSOURCE_BIT     (2U)
#define COUNT_FLAG_BIT   (16U)
#define DEFAULT_TIMER_VAL (0U)

void SysTick_Run(unsigned int msec)
{
	// Timer 설정 : 인터럽트 발생 안함, clock source는 HCLK/8, Timer 정지 (enable 시키지 말것)
	// TICKINT(bit 1): 0, CLKSOURCE(bit 2): 0, ENABLE(bit 0) : 0
	// 초기 설정하는 레지스터는 대입이 주로 사용됨 (다른 비트까지 초기화)
	SysTick->CTRL = (0 << ENABLE_BIT) | (0 << TICKINT_BIT) | (0 << CLKSOURCE_BIT);
	// Macro_Write_Block(SysTick->CTRL, 0x7, (0 << ENABLE_BIT)|(0 << TICKINT_BIT)|(0 << CLKSOURCE_BIT), 0);

	// 주어진 msec 값 만큼의 msec를 count하는 초기값 설정 (LOAD)
	// HCLK을 나누었을 때 double로 받기 위해 8.
	// 정밀한 측정을 위해 +0.5 후 int형으로 변환함으로써 반올림
	SysTick->LOAD = msec * (int)(HCLK / (8. * 1000) + 0.5);

	// SysTick->LOAD = msec * HCLK / (8. * 1000.);
	// 92000000 * 1000 연산 시 정수형 범위를 벗어나서 ()를 삽입했을 때와 안했을 때 연산 결과 차이가 난다.

	// VAL 레지스터 값 초기화(0) 및 COUNTFLAG Clear
	// VAL에 임의의 값을 쓰면 COUNTFLAG가 clear됨
	SysTick->VAL = DEFAULT_TIMER_VAL;

	// Timer Start (시작이 되면 자동으로 LOAD의 값을 VAL로 가져간다)
	Macro_Set_Bit(SysTick->CTRL, ENABLE_BIT);
}

int SysTick_Check_Timeout(void)
{
	// Timer의 Timeout이 발생하면 참(1)리턴, 아니면 거짓(0) 리턴
	return (Macro_Check_Bit_Set(SysTick->CTRL, COUNT_FLAG_BIT));
}

unsigned int SysTick_Get_Time(void)
{
	// Timer의 현재 count 값 리턴
	return (SysTick->VAL);
}

unsigned int SysTick_Get_Load_Time(void)
{
	// Timer에 설정된 초기값을 리턴
	return (SysTick->LOAD);
}

void SysTick_Stop(void)
{
	// Timer Stop -> ENABLE 0
	Macro_Clear_Bit(SysTick->CTRL, ENABLE_BIT);
}
