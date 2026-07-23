#include "device_driver.h"

/* ========== Timer 2 ========== */
void TIM2_Repeat_Interrupt_Enable(int en, int time)
{
	if(en)
	{
		// TIM4 Clock On
		Macro_Set_Bit(RCC->APB1ENR, 0);

		TIM2->CR1 = (1<<4)|(0<<3);
		TIM2->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
		TIM2->ARR = TIM2_1ms_Pls * time;
		Macro_Set_Bit(TIM2->EGR,0);

		// TIM4 Pending Clear (bit 0 UIF Clear)
		Macro_Clear_Bit(TIM2->SR, UIF_BIT);

		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(TIM2_IRQn);

		// TIM4 Interrupt Enable (bit 0 UIE Set)
		Macro_Set_Bit(TIM2->DIER, UIE_BIT);

		// NVIC Interrupt Enable
		NVIC_EnableIRQ(TIM2_IRQn);

		// TIM4 Start (bit 0 CEN bit Set)
		Macro_Set_Bit(TIM2->CR1, CEN_BIT);
	}

	else
	{
		NVIC_DisableIRQ(TIM2_IRQn);
		Macro_Clear_Bit(TIM2->CR1, 0);
		Macro_Clear_Bit(TIM2->DIER, 0);
	}
}

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

void TIM2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);
	Macro_Clear_Bit(TIM2->DIER, 0);
}

/* ========== Timer 4 ========== */
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

void TIM4_Repeat_Interrupt_Enable(int en, int time)
{
	if(en)
	{
		// TIM4 Clock On
		Macro_Set_Bit(RCC->APB1ENR, 2);

		TIM4->CR1 = (1<<4)|(0<<3);
		TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
		TIM4->ARR = TIM4_1ms_Pls * time;
		Macro_Set_Bit(TIM4->EGR,0);

		// TIM4 Pending Clear (bit 0 UIF Set)
		Macro_Clear_Bit(TIM4->SR, UIF_BIT);

		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(TIM4_IRQn);

		// TIM4 Interrupt Enable (bit 0 UIE Set)
		Macro_Set_Bit(TIM4->DIER, UIE_BIT);

		// NVIC Interrupt Enable
		NVIC_EnableIRQ(TIM4_IRQn);

		// TIM4 Start (bit 0 CEN bit Set)
		Macro_Set_Bit(TIM4->CR1, CEN_BIT);
	}

	else
	{
		NVIC_DisableIRQ(30);
		Macro_Clear_Bit(TIM4->CR1, 0);
		Macro_Clear_Bit(TIM4->DIER, 0);
	}
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
	Macro_Clear_Bit(TIM4->DIER, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = 50 * time;
}

/* ========== Timer 3 ========== */
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

void TIM3_Out_Freq_Generation(unsigned short freq, uint8_t speed_level)
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

	TIM3->CR1 = (0x1 << 4) | (0x0 << 3);
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	// 96MHz / 8MHz
	TIM3->PSC = TIMXCLK / TIM3_FREQ - 1;

	// 요청한 주파수가 되도록 ARR 설정
	// 8MHz / target_freq
	TIM3->ARR = (int)((double)TIM3_FREQ / freq + 0.5) -1 ;

	// Duty Rate 50%가 되도록 CCR3 설정
	TIM3->CCR3 = ((TIM3->ARR + 1U) * duty_percent / 100U);

	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM3->EGR, UG_BIT);

	// Down Counter, Repeat Mode, Timer Start
	TIM3->CR1 = (DIR_DOWN << DIR_BIT) | (OPM_REPEAT << OPM_BIT) | (CEN_ENABLE << CEN_BIT);
}

int Calculate_PWM_CCR(uint8_t speed_level)
{
	int duty_percent = 50U + ((speed_level - 1U) * 5U);
	return ((TIM3_PWM_ARR + 1U) * duty_percent / 100U);
}