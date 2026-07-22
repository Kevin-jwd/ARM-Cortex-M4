#include "device_driver.h"


void TIM2_Stopwatch_Start(void)
{
	// APB1 주변장치 clock on (bit 0 : TIM2 EN)
	Macro_Set_Bit(RCC->APB1ENR, TIM2_CLK_EN_BIT);

	// TIM2 CR1 설정: down count(ARPE : 0, DIR : 1), one shot (OPM : 1), disable (CEN : 0)
	TIM2->CR1 = (ARPE_DOWN << ARPE_BIT) | (DIR_DOWN << DIR_BIT) | (OPM_ONE_SHOT << OPM_BIT) | (CEN_DISABLE << CEN_BIT);

	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = (int)(DEFAULT_PSC + 0.5)-1;

	// ARR 초기값 설정 => 최대값 0xFFFF 설정
	TIM2->ARR = 0xFFFF;

	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, UG_BIT);

	// TIM2 start (counter enable)
	Macro_Set_Bit(TIM2->CR1, CEN_BIT);
}

unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	// TIM2 stop (counter disable)
	Macro_Clear_Bit(TIM2->CR1, CEN_BIT);

	// CNT 초기 설정값 (0xffff)와 현재 CNT의 펄스수 차이 (DEFAULT_ARR - TIM2->CNT) 를 구하고
	// 그 펄스수 하나가 20usec이므로 20을 곱한값을 time에 저장
	time = (DEFAULT_ARR - TIM2->CNT) * TIM2_TICK;

	// 계산된 time 값을 리턴(단위는 usec)
	return time;
}

#if 0
void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, TIM2_CLK_EN_BIT);
	// TIM2 CR1 설정: down count(ARPE : 0, DIR : 1), one shot (OPM : 1), disable (CEN : 0)
	TIM2->CR1 = (ARPE_DOWN << ARPE_BIT) | (DIR_DOWN << DIR_BIT) | (OPM_ONE_SHOT << OPM_BIT) | (CEN_DISABLE << CEN_BIT);

	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = (int)(DEFAULT_PSC + 0.5)-1;

	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	TIM2->ARR = time * (int)(1000./TIM2_TICK + 0.5);

	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, UG_BIT);

	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR, UIF_BIT);

	// TIM2 start
	Macro_Set_Bit(TIM2->CR1, CEN_BIT);

	// Wait timeout
	while(!Macro_Check_Bit_Set(TIM2->SR, UIF_BIT));

	// manual clear
	Macro_Clear_Bit(TIM2->SR, UIF_BIT);

	// TIM2 Stop
	Macro_Clear_Bit(TIM2->CR1, CEN_BIT);
}
#endif

#if 1
void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);
	TIM2->CR1 = (0x1<<4)|(0x1<<3);
	TIM2->PSC = (int)(TIMXCLK / TIM2_FREQ + 0.5) - 1;

	unsigned int pls = TIM2_1ms_Pls * time;
	int n = pls / TIM2_MAX;
	int m = pls % TIM2_MAX;
	int i;

	for(i = 0; i < n; i++)
	{
		TIM2->ARR = TIM2_MAX;
		Macro_Set_Bit(TIM2->EGR, 0);
		Macro_Clear_Bit(TIM2->SR, 0);
		Macro_Set_Bit(TIM2->CR1, 0);
		while(!Macro_Check_Bit_Set(TIM2->SR, 0));		
	}

	TIM2->ARR = m;
	Macro_Set_Bit(TIM2->EGR, 0);
	Macro_Clear_Bit(TIM2->SR, 0);
	Macro_Set_Bit(TIM2->CR1, 0);
	while(!Macro_Check_Bit_Set(TIM2->SR, 0));

	Macro_Clear_Bit(TIM2->CR1, 0);
}
#endif

void TIM2_Start_OnePulse(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, TIM2_CLK_EN_BIT);

	// TIM4 CR1: ARPE=0, down counter, repeat mode
	TIM2->CR1 = (ARPE_DOWN << ARPE_BIT) | (DIR_DOWN << DIR_BIT) | (OPM_ONE_SHOT << OPM_BIT) | (CEN_DISABLE << CEN_BIT);

	// PSC(50KHz),  ARR 설정
    TIM2->PSC = (int)(DEFAULT_PSC + 0.5) - 1;

	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
    TIM2->ARR = time * (int)(1000. / TIM2_TICK + 0.5);

	// UG 이벤트 발생
    Macro_Set_Bit(TIM2->EGR, UG_BIT);

	// Update Interrupt Pending Clear
    Macro_Clear_Bit(TIM2->SR, UIF_BIT);

	// TIM4 start
    Macro_Set_Bit(TIM2->CR1, CEN_BIT);
}

int TIM2_Check_Timeout(void)
{
	if(Macro_Check_Bit_Set(TIM2->SR, UIF_BIT))
	{
		Macro_Clear_Bit(TIM2->SR, UIF_BIT);
		return 1;
	}
	else
	{
		return 0;
	}
}

void TIM4_Repeat(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, TIM4_CLK_EN_BIT);

	// TIM4 CR1: ARPE=0, down counter, repeat mode
	TIM4->CR1 = (ARPE_DOWN << ARPE_BIT) | (DIR_DOWN << DIR_BIT) | (OPM_REPEAT << OPM_BIT) | (CEN_DISABLE << CEN_BIT);

	// PSC(50KHz),  ARR(reload시 값) 설정
	TIM4->PSC = (int)(DEFAULT_PSC + 0.5)-1;

	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	TIM4->ARR = time * (int)(1000./TIM4_TICK + 0.5);

	// UG 이벤트 발생
	Macro_Set_Bit(TIM4->EGR, UG_BIT);

	// Update Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR, UIF_BIT);

	// TIM4 start
	Macro_Set_Bit(TIM4->CR1, CEN_BIT);
}

int TIM4_Check_Timeout(void)
{
	if(Macro_Check_Bit_Set(TIM4->SR, UIF_BIT))
	{
		Macro_Clear_Bit(TIM4->SR, UIF_BIT);
		return 1;
	}
	else
	{
		return 0;
	}
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = 50 * time;
}
void TIM3_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 1);
	Macro_Set_Bit(RCC->APB1ENR, 1);
	
	Macro_Write_Block(GPIOB->MODER, 0x3, 0x2, 0);  	// PB0 => ALT
	Macro_Write_Block(GPIOB->AFR[0], 0xf, 0x2, 0); 	// PB0 => AF02
	
	Macro_Write_Block(TIM3->CCMR2,0xff, 0x60, 0);
	TIM3->CCER = (0<<9)|(1<<8);
	}

void TIM3_Out_Stop(void)
{
	Macro_Clear_Bit(TIM3->CR1, 0);
}


void TIM3_Out_PWM_Generation(unsigned short freq, int duty)
{
	TIM3->CR1 = (0x1 << 4) | (0x0 << 3);

	TIM3->PSC = TIMXCLK / TIM3_FREQ - 1;

	TIM3->ARR = (int)((double)TIM3_FREQ / freq + 0.5) -1 ;

	// Duty Rate 50%가 되도록 CCR3 설정
	TIM3->CCR3 = (int)(TIM3->ARR * ((double)duty / 100.) + 0.5);

	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM3->EGR, UG_BIT);

	// Down Counter, Repeat Mode, Timer Start
	TIM3->CR1 = (DIR_DOWN << DIR_BIT) | (OPM_REPEAT << OPM_BIT) | (CEN_ENABLE << CEN_BIT);
}

void TIM3_Out_Freq_Generation(unsigned short freq)
{
	TIM3->CR1 = (0x1 << 4) | (0x0 << 3);
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	// 96MHz / 8MHz
	TIM3->PSC = TIMXCLK / TIM3_FREQ - 1;

	// 요청한 주파수가 되도록 ARR 설정
	// 8MHz / target_freq
	TIM3->ARR = (int)((double)TIM3_FREQ / freq + 0.5) -1 ;

	// Duty Rate 50%가 되도록 CCR3 설정
	TIM3->CCR3 = TIM3->ARR / 2;

	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM3->EGR, UG_BIT);

	// Down Counter, Repeat Mode, Timer Start
	TIM3->CR1 = (DIR_DOWN << DIR_BIT) | (OPM_REPEAT << OPM_BIT) | (CEN_ENABLE << CEN_BIT);
}

int Calculate_PWM_CCR(uint8_t speed_level)
{
	if(speed_level < 1U)
	{
		speed_level = 1U;
	}
	else if(speed_level > 9U)
	{
		speed_level = 9U;
	}

	int duty_percent = 50U + ((speed_level - 1U) * 5U);
	return ((TIM3_PWM_ARR + 1U) * duty_percent / 100U);
}