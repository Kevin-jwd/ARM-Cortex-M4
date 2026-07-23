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
static void Turn_Clockwise_PWM(void);
static void Turn_Counter_Clockwise_PWM(void);

// Motor State enumerate
typedef enum t_MOTOR_STATE{
	TURN_CLOCKWISE          = 0U,
	TURN_COUNTER_CLOCKWISE  = 1U,
	STOP                    = 2U,
	SHORT_STOP              = 3U
} MOTOR_STATE;

// Button State enumerate
typedef enum t_BTN_STATE{
	IS_RELEASED = 0U,
	IS_PRESSED  = 1U
} BTN_STATE;

// 타이머 시작, 타이머 타임아웃 플래그 초기화
volatile uint8_t is_timer_started = 0U;
volatile uint8_t is_timer_timeout = 0U;
volatile uint8_t is_stop_delay_timeout = 0U;
volatile static uint8_t is_long_press = 0U;
volatile int is_falling_edge_detected = 0U;

volatile uint8_t Uart_Data_In = 0U;
volatile unsigned char Uart_Data = 0;

/* Main */
void Main(void)
{
	// Initialization
	Sys_Init(BAUD_RATE);
	Key_ISR_Enable(1);

	GPIO_Init();
	Stop();
	
	printf("Motor Start\n");
	
	// 상태 변수 초기화
	MOTOR_STATE motor_state = STOP;
	MOTOR_STATE target_state = STOP; 
	BTN_STATE curr_btn_state = IS_RELEASED;
	BTN_STATE prev_btn_state = curr_btn_state;

	// FSM 진입
	for(;;)
	{
		// char received_data = Uart2_Get_Pressed();
		// if((received_data >= '1') && (received_data <= '9'))
		// {
		// 	TIM3_Out_Freq_Generation(10000U, received_data - 0);
		// }

		// Falling Edge Detection (prev: H, curr: L)
		// MISRA C: 조건문에서 비트 연산자 (&) 대신 논리 연산자 (&&) 사용
		if(is_falling_edge_detected && !is_timer_started)
		{
			printf("Falling Edge!\n");
			TIM2_Repeat_Interrupt_Enable(1, STOP_MSEC);
			is_timer_started = 1U;
			is_long_press = 0U;
			printf("Timer Start!\n");
		}
				
		// Rising Edge Detection (prev: L, curr: H)
		if(!is_falling_edge_detected && is_timer_started)
		{
			printf("Rising Edge!\n");
			is_timer_started = 0U;
			TIM2_Stop();

			// long pressed
			if(is_long_press)
			{
				is_long_press = 0U;
				is_timer_timeout = 0U;
			}
			// short pressed
			else
			{
				if(motor_state == STOP)
				{
					motor_state = TURN_CLOCKWISE;
					printf("Motor State Changed: %d\n", motor_state);
				}
				else if(motor_state != SHORT_STOP)
				{
					target_state = 1U - motor_state;
					motor_state = SHORT_STOP;
					TIM4_Repeat_Interrupt_Enable(1, STOP_DELAY_MSEC); 
					printf("Stop Delay Start! Target: %d\n", target_state);
				}
			}
		}

		// 버튼 길게 누름: 3000msec timeout
		if(is_timer_timeout && motor_state != STOP)
		{
			motor_state = STOP;
			is_timer_timeout = 0U;
			is_long_press = 1U;
			TIM2_Stop();
			printf("Timer Timeout! Motor State Changed: %d\n", motor_state);
		}

		// 정지딜레이: 300msec timeout
		if(is_stop_delay_timeout)
		{
			motor_state = target_state;
			is_stop_delay_timeout = 0U;
			TIM4_Stop();
			printf("Stop Delay Done! Motor State Changed: %d\n", motor_state);
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
			case SHORT_STOP: 
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
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
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
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_COUNTER_CLOCKWISE_SIGNAL, GPIOA0_PIN);
}

/* Motor Stop function */
static void Stop(void)
{
	Macro_Write_Block(GPIOA->ODR, TWO_BIT, STOP_SIGNAL, GPIOA0_PIN);
}

static void Turn_Clockwise_PWM(void)
{
	Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 0);     // PA0 => ALT
  	Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0);  // PA2,3 => AF07
	Macro_Clear_Bit(GPIOA->ODR, GPIOA1_PIN);
	// Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_CLOCKWISE_SIGNAL, GPIOA0_PIN);
}

static void Turn_Counter_Clockwise_PWM(void)
{
	Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 4);     // PA2,3 => ALT
  	Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 4);  // PA2,3 => AF07
	Macro_Clear_Bit(GPIOA->ODR, GPIOA0_PIN);
	// Macro_Write_Block(GPIOA->ODR, TWO_BIT, TURN_CLOCKWISE_SIGNAL, GPIOA0_PIN);
}

