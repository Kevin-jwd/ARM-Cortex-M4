#include "device_driver.h"
#include <stdio.h>

// Basic settings
#define BAUD_RATE       (115200U)
#define STOP_MSEC         (3000U)
#define STOP_DELAY_MSEC    (300U)
#define MOTOR_STATE_CNT  (2U)
#define MOTOR_FREQ      (10000U)

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
#define STOP_SIGNAL                    (0x0U)

// Function declaration
static void Sys_Init(int baud);
static void GPIO_Init(void);
static void Turn_Clockwise(void);
static void Turn_Counter_Clockwise(void);
static void Stop(void);
static void Turn_Clockwise_PWM(int data);
static void Turn_Counter_Clockwise_PWM(int data);
static void PORTA_Mode(int port);

// Motor State enumerate
typedef enum t_MOTOR_STATE{
	TURN_CLOCKWISE             = 0U,
	TURN_COUNTER_CLOCKWISE     = 1U,
	TURN_CLOCKWISE_PWM         = 2U,
	TURN_COUNTER_CLOCKWISE_PWM = 3U,
	STOP                       = 4U,
	SHORT_STOP                 = 5U
} MOTOR_STATE;

// Motor direction
typedef enum t_MOTOR_DIR{
	CLOCKWISE          = 0U,
	COUNTER_CLOCKWISE  = 1U
} MOTOR_DIR;

// Button State enumerate
typedef enum t_BTN_STATE{
	IS_RELEASED = 0U,
	IS_PRESSED  = 1U
} BTN_STATE;

// Button 플래그
volatile uint8_t is_falling_edge_detected = 0U;
volatile static uint8_t is_long_pressed = 0U;

// Timer 플래그
volatile uint8_t is_timer2_started = 0U;
volatile uint8_t is_timer2_timeout = 0U;
volatile uint8_t is_timer4_timeout = 0U;

// Uart 플래그
volatile uint8_t Uart_Data_In = 0U;
volatile unsigned char Uart_Data = 0U;

int speed = 0U;

/* Main */
void Main(void)
{
	// Initialization
	Sys_Init(BAUD_RATE);

	// ISR Enable
	Key_ISR_Enable(1);
	Uart2_RX_Interrupt_Enable(1);

	GPIO_Init();
	Stop();
	
	printf("Motor Start\n");
	
	// 상태 변수 초기화
	MOTOR_STATE  motor_state = STOP;

	MOTOR_DIR curr_dir = CLOCKWISE;
	MOTOR_DIR target_dir = CLOCKWISE;

	BTN_STATE curr_btn_state = IS_RELEASED;
	BTN_STATE prev_btn_state = curr_btn_state;

	// FSM 진입
	for(;;)
	{
		if(Uart_Data_In)
		{
			Uart_Data_In = 0U;
			if((Uart_Data >= '1') && (Uart_Data <= '9'))
			{
				if(motor_state == TURN_CLOCKWISE || motor_state == TURN_COUNTER_CLOCKWISE)
				{
					speed = Uart_Data - 0;
					if(curr_dir == CLOCKWISE)
					{
						motor_state = TURN_CLOCKWISE;
						printf("속도 제어 : 시계방향 %c\n", speed);
					}
					else if(curr_dir == COUNTER_CLOCKWISE)
					{
						motor_state = TURN_COUNTER_CLOCKWISE;
						printf("속도 제어 : 반시계방향 %c\n", speed);
					}
				}
			}
		}

		// Falling Edge Detection (prev: H, curr: L)
		// MISRA C: 조건문에서 비트 연산자 (&) 대신 논리 연산자 (&&) 사용
		if(is_falling_edge_detected && (!is_timer2_started))
		{
			TIM2_Repeat_Interrupt_Enable(1U, STOP_MSEC);
			printf("Falling Edge!\n");
			printf("Timer Start!\n");
			is_timer2_started = 1U;
			is_long_pressed = 0U;
		}
				
		// Rising Edge Detection (prev: L, curr: H)
		if(!is_falling_edge_detected && is_timer2_started)
		{
			TIM2_Stop();
			printf("Rising Edge!\n");
			is_timer2_started = 0U;

			// long pressed
			if(is_long_pressed)
			{
				is_long_pressed = 0U;
				is_timer2_timeout = 0U;
			}
			// short pressed
			else if(motor_state == STOP)
			{
				curr_dir = CLOCKWISE;
				motor_state = TURN_CLOCKWISE;

				printf("Motor State Changed: %d\n", motor_state);
			}
			else if(motor_state != SHORT_STOP)
			{
				TIM4_Repeat_Interrupt_Enable(1U, STOP_DELAY_MSEC); 
				target_dir = 1U - curr_dir;
				printf("target_dir : %d\n", target_dir);
				motor_state = SHORT_STOP;
				printf("Stop Delay Start! Target: %d\n", target_dir);
			}
		}

		// 3초 이상 버튼 눌러 종료: 3000msec timeout
		if(is_timer2_timeout && (motor_state != STOP))
		{
			GPIO_Init();
			TIM2_Stop();
			is_timer2_timeout = 0U;
			is_long_pressed = 1U;

			motor_state = STOP;
			printf("Timer Timeout! Motor State Changed: %d\n", motor_state);
		}

		// 토글 전 정지: 300msec timeout
		if(is_timer4_timeout)
		{
			TIM4_Stop();
			curr_dir = target_dir;
			if(curr_dir == CLOCKWISE)
			{
				motor_state = TURN_CLOCKWISE;
			}
			else if(curr_dir == COUNTER_CLOCKWISE)
			{
				motor_state = TURN_COUNTER_CLOCKWISE;
			}

			is_timer4_timeout = 0U;

			printf("Stop Delay Done! Motor State Changed: %d\n", motor_state);
		}

		// motor_state에 따라 Action
		switch(motor_state)
		{
			case TURN_CLOCKWISE:
				PORTA_Mode(0);
				Turn_Clockwise();
				break;
			case TURN_COUNTER_CLOCKWISE:
				PORTA_Mode(1);
				Turn_Counter_Clockwise();
				break;
			case TURN_CLOCKWISE_PWM:
				
				Turn_Clockwise_PWM(speed);
				break;
			case TURN_COUNTER_CLOCKWISE_PWM:
				PORTA_Mode(1);
				Turn_Counter_Clockwise_PWM(speed);
				break;	
			case STOP:
			case SHORT_STOP: 
				PORTA_Mode(2);
				Stop();
				break;
			default:
				PORTA_Mode(2);
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
	// GPIOA 
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

static void Turn_Clockwise_PWM(int data)
{
	TIM5_Out_Init();
	Macro_Write_Block(TIM5->CCMR1,0xff, 0x60, 0);     // 타이머 5 채널 1 cc 설정
	Macro_Clear_Bit(GPIOA->ODR, GPIOA1_PIN);          // PA1 => General Output 0
	TIM5_Out_PWM_Generation(MOTOR_FREQ, data, TURN_CLOCKWISE);
}

static void Turn_Counter_Clockwise_PWM(int data)
{
	TIM5_Out_Init();
	Macro_Write_Block(TIM5->CCMR2,0xff, 0x60, 8);     // 타이머 5 채널 2 cc 설정
	Macro_Clear_Bit(GPIOA->ODR, GPIOA0_PIN);          // PA0 => General Output 0
	TIM5_Out_PWM_Generation(MOTOR_FREQ, data, TURN_COUNTER_CLOCKWISE);
}

static void PORTA_Mode(int port)
{
	// CW
	if (port == 0)
    {
        TIM5->CCR2 = 0;
        Macro_Write_Block(GPIOA->MODER, 0x03,0x02,0); // altimate function mode
        Macro_Write_Block(GPIOA->MODER, 0x03,0x01,2);
        Macro_Clear_Bit(GPIOA->ODR,1);
    }
	// CCW
    else if (port == 1)
    {
        TIM5->CCR1 = 0;
        Macro_Write_Block(GPIOA->MODER, 0x03,0x02,2); // altimate function mode
        Macro_Write_Block(GPIOA->MODER, 0x03,0x01,0);
        Macro_Clear_Bit(GPIOA->ODR,0);
    }
	// STOP | default
    else if (port == 2)
    {
        TIM5->CCR1 = 0;
        TIM5->CCR2 = 0;
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,0);
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,2);
    }
}

