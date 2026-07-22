#include "device_driver.h"
#include <stdio.h>

// Basic settings
#define BAUD_RATE       (115200U)
#define STOP_MSEC         (3000U)
#define STOP_DELAY_MSEC    (300U)
#define MOTOR_STATE_CNT  (2U)

// Bit position
#define TWO_BIT            (0x3U)
#define FOUR_BIT           (0xFU)

// GPIO Mode
#define PUSH_PULL_MODE      (0x0U)
#define GENERAL_OUTPUT_MODE (0x1U)

#define FOUR_BIT_GENERAL_PURPOSE_OUTPUT_MODE (0x5U)
#define TWO_BIT_OUTPUT_PUSH_PULL_MODE        (0x0U)

// GPIO PIN (PA0, PA1, PC13)
#define GPIOA0_PIN           (0U)
#define GPIOA1_PIN           (1U)
#define GPIOC_PIN           (13U)

// Motor Pin Signal
#define TURN_CLOCKWISE_SIGNAL          (0x2U)
#define TURN_COUNTER_CLOCKWISE_SIGNAL  (0x1U)
#define STOP_SIGNAL       (0x0U)

// Function declaration
static void Sys_Init(int baud);
static void GPIO_Init(void);
static void Turn_Clockwise(void);
static void Turn_Counter_Clockwise(void);
static void Stop(void);

// Motor State enumerate
typedef enum t_MOTOR_STATE{
	TURN_CLOCKWISE          = 0U,
	TURN_COUNTER_CLOCKWISE  = 1U,
	STOP                    = 2U
} MOTOR_STATE;

// Button State enumerate
typedef enum t_BTN_STATE{
	IS_RELEASED = 0U,
	IS_PRESSED  = 1U
} BTN_STATE;

/* Main */
void Main(void)
{
	// Initialization
	Sys_Init(BAUD_RATE);
	Key_Poll_Init();
	GPIO_Init();
	Stop();
	printf("Motor Start\n");
	
	// 상태 변수 초기화
	MOTOR_STATE motor_state = STOP;
	BTN_STATE curr_btn_state = IS_RELEASED;
	BTN_STATE prev_btn_state = curr_btn_state;

	// 타이머 시작, 타이머 타임아웃 플래그 초기화
	uint8_t is_timer_started = 0U;
	uint8_t is_timer_timeout = 0U;

	// FSM 진입
	for(;;)
	{
		char received_data = Uart2_Get_Pressed();
		if((received_data >= '1') && (received_data <= '9'))
		{
			;
		}
		prev_btn_state = curr_btn_state;
		curr_btn_state = Key_Get_Pressed();

		// Falling Edge Detection (prev: H, curr: L)
		// MISRA C: 조건문에서 비트 연산자 (&) 대신 논리 연산자 (&&) 사용
		if((prev_btn_state == IS_RELEASED) && (curr_btn_state == IS_PRESSED))
		{
			is_timer_timeout = 0;
			TIM2_Start_OnePulse(STOP_MSEC);
			is_timer_started = 1;
		}

		// Rising Edge Detection (prev: L, curr: H)
		if((prev_btn_state == IS_PRESSED) && (curr_btn_state == IS_RELEASED))
		{
			is_timer_started = 0;
			if(is_timer_timeout)
			{
				motor_state = STOP;
			}
			else
			{
				motor_state = (motor_state == STOP) ? TURN_CLOCKWISE : (1U - motor_state);
			}
		}

		// 버튼 눌린 채로 3초 지남
		if(TIM2_Check_Timeout() & is_timer_started)
		{
			is_timer_timeout = 1;
			is_timer_started = 0;
			motor_state = STOP;
		}

		// motor_state에 따라 Action
		switch(motor_state)
		{
			case TURN_CLOCKWISE:
				Turn_Clockwise();
				break;
			case TURN_COUNTER_CLOCKWISE:
				Turn_Counter_Clockwise();
				break;
			case STOP:
				Stop();
				break;
			default:
				Stop();
				break;
		}
	}
}

/* Initialize Timer, Uart, LED */
static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

/* Initialize GPIO Pins */
static void GPIO_Init(void)
{
	// PA0, PA1 : General purpose output mode (0101)
	Macro_Write_Block(GPIOA->MODER, FOUR_BIT, FOUR_BIT_GENERAL_PURPOSE_OUTPUT_MODE, (GPIOA0_PIN * 2U));
	
	// PA0, PA1 : Output push-pull (00)
	Macro_Write_Block(GPIOA->OTYPER, TWO_BIT, TWO_BIT_OUTPUT_PUSH_PULL_MODE, GPIOA0_PIN);
}

/* Motor Turn Clockwise function */
static void Turn_Clockwise(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_CLOCKWISE_SIGNAL, GPIOA0_PIN);
}

/* Motor Turn Counter Clockwise function */
static void Turn_Counter_Clockwise(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_COUNTER_CLOCKWISE, GPIOA0_PIN);
}

/* Motor Stop function */
static void Stop(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, STOP_SIGNAL, GPIOA0_PIN);
}