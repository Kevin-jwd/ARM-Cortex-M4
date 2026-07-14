#if 1
#define  GPIOA_MODER (*(unsigned long*)0x40020000)
#define GPIOA_OTYPER (*(unsigned long*)0x40020004)
#define    GPIOA_ODR (*(unsigned long*)0x40020014)		

void Main(void)
{
	// LED GPA[7] Open drain 모드 설정
	// 15bit 14bit
	GPIOA_MODER  |= 0x1 << 14;
	GPIOA_OTYPER |= 0x1 << 7;

	// GPA[7] LED ON
	GPIOA_ODR    |= 0x0 << 7;
}
#endif

#if 0
#define  GPIOA_MODER (*(unsigned long*)0x40020000)
#define GPIOA_OTYPER (*(unsigned long*)0x40020004)
#define    GPIOA_ODR (*(unsigned long*)0x40020014)		

void Main(void)
{
	// LED GPA[5] General Push-pull 모드 설정
	GPIOA_MODER  = 0x1 << 10;
	GPIOA_OTYPER = 0x0 << 5;

	// GPA[5] LED ON
	GPIOA_ODR    = 0x1 << 5;
}
#endif