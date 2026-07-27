#include "device_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void Uart1_Init(int baud)
{
  double div;
  unsigned int mant;
  unsigned int frac;

  Macro_Set_Bit(RCC->AHB1ENR, 0);                   // PA9,10
  Macro_Set_Bit(RCC->APB2ENR, 4);                   // USART1 ON
  Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 18);    // PA9,10 => ALT

  // AFR[0]: 각 포트의  0~7번 핀에 대한 주변장치 설정
  // AFR[1]: 각 포트의 8~15번 핀에 대한 주변장치 설정
  // 아래는 9, 10번 핀에 대해 AF07 선택
  Macro_Write_Block(GPIOA->AFR[1], 0xff, 0x77, 4);  // PA9,10 => AF07

  Macro_Write_Block(GPIOA->PUPDR, 0xf, 0x5, 18);    // PA9,10 => Pull-Up

  div = PCLK2 / (16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16 + 0.5);
  mant += frac >> 4;
  frac &= 0xf;
  USART1->BRR = (mant<<4)|(frac<<0);

  // CR1: USART 기본 동작 설정
  // *UE (bit 13): USART Enable (0: disable, 1: enable)
  // *M  (bit 12): word length (0: 8bit, 1: 9bit)
  // *PCE (bit 10): parity control enable (0: disable, 1: enable)
  // PEIE (bit 8): parity err interrupt enable
  // TXEIE (bit 7): tx empty interrupt enable
  // TCIE (bit 6): tx complete interrupt enable
  // RXNEIE (bit 5): rx not empty interrupt enable
  // *TE (bit 3): TX enable
  // *RE (bit 2): RX enable
  USART1->CR1 = (1<<13)|(0<<12)|(0<<10)|(1<<3)|(1<<2);

  // CR2: 주로 LIN에 관한 USART의 설정 담당
  // STOP (bit 13, 12): 00: 1 (TeraTerm 설정)
  USART1->CR2 = 0 << 12;
  USART1->CR3 = 0;
}

void Uart1_Send_Byte(char data)
{
  if(data == '\n')
  {
    // SR: USART의 상태 확인
    // TXE (bit 7): TX data register empty (1: empty, DR 레지스터에 write하면 자동 clear)
    // RXNE (bit 5): RX data not emtpy (1: full, DR 레지스터 read하면 자동 clear)
    while(!Macro_Check_Bit_Set(USART1->SR, 7));

    // DR: USART의 송수신 데이터를 읽거나 쓰는 역할
    USART1->DR = 0x0d;
  }

  while(!Macro_Check_Bit_Set(USART1->SR, 7));
  USART1->DR = data;
}

void Uart1_Send_String(char *pt)
{
  while(*pt != 0)
  {
    Uart1_Send_Byte(*pt++);
  }
}

void Uart2_Init(int baud)
{
  double div;
  unsigned int mant;
  unsigned int frac;

  Macro_Set_Bit(RCC->AHB1ENR, 0);                   // PA2,3
  Macro_Set_Bit(RCC->APB1ENR, 17);                   // USART2 ON
  Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 4);     // PA2,3 => ALT
  Macro_Write_Block(GPIOA->AFR[0], 0xff, 0x77, 8);  // PA2,3 => AF07
  Macro_Write_Block(GPIOA->PUPDR, 0xf, 0x5, 4);     // PA2,3 => Pull-Up  

  volatile unsigned int t = GPIOA->LCKR & 0x7FFF;
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<2);                // Lock PA2, 3 Configuration
  GPIOA->LCKR = (0x0<<16)|t|(0x3<<2);
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<2);
  t = GPIOA->LCKR;

  div = PCLK1/(16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16. + 0.5);
  mant += frac >> 4;
  frac &= 0xf;

  USART2->BRR = (mant<<4)|(frac<<0);
  USART2->CR1 = (1<<13)|(0<<12)|(0<<10)|(1<<3)|(1<<2);
  USART2->CR2 = 0<<12;
  USART2->CR3 = 0;
}

void Uart2_Send_Byte(char data)
{
  if(data == '\n')
  {
    while(!Macro_Check_Bit_Set(USART2->SR, 7));
    USART2->DR = 0x0d;
  }

  while(!Macro_Check_Bit_Set(USART2->SR, 7));
  USART2->DR = data;
}

