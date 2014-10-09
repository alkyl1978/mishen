#include "stm32f0xx.h" 
#include "uart.h"
#include "nrf24l01.h"

volatile unsigned char RXbuf[2][8]; 
volatile unsigned char nRXpr;       
volatile unsigned char nRX;               
volatile unsigned char fRXpr;      
volatile unsigned char TXbuf[7];   

extern volatile int8_t ftx;    // флаг указателя передачи пакета
extern RF_TypeDef RF;


void Uart_init(void) 
{
   nRXpr=0; 
   RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_DMA1EN; // включаем тактирование DMA GPIOA
   RCC->APB2ENR|=RCC_APB2ENR_USART1EN; // включаем тактирование USART
   GPIOA->MODER &= ~(GPIO_MODER_MODER9 |GPIO_MODER_MODER10); // сбрасываем настройки портов
   GPIOA->MODER |=  GPIO_MODER_MODER10_1 |GPIO_MODER_MODER9_1;  // настраиваем режим работы
   GPIOA->AFR[1]|= (1<<(4*1)) |(1<<(4*2)); // включаем альтернативные функции
   USART1->BRR =(APBCLK+BAUDRATE/2)/BAUDRATE;  // вычисляем битрейд
	 // конфигурируем USART
	 USART1->CR1 |= USART_CR1_RE;
	 USART1->CR1 |= USART_CR1_TE;	 
	 USART1->CR1 |= USART_CR1_IDLEIE;  // 
   USART1->CR3|=USART_CR3_DMAR|USART_CR3_DMAT;  
	 //**************************************************************************
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // включаем модуль
	// ремапим DMA каналы
	 SYSCFG->CFGR1|=SYSCFG_CFGR1_USART1TX_DMA_RMP | SYSCFG_CFGR1_USART1RX_DMA_RMP;
	 //настраиваем ДМА для передачи пакета
	 DMA1_Channel4->CPAR=(uint32_t)&USART1->TDR; // настраиваем буфер ДМА от куда пересылать
	 DMA1_Channel4->CNDTR=7; // количество байт
	 DMA1_Channel4->CMAR=(uint32_t)&TXbuf[0]; // передающий буфер 
	 DMA1_Channel4->CCR=0; // сбрасываем регист
	 DMA1_Channel4->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE|DMA_CCR_DIR; // настраиваем регистр 
	 DMA1_Channel4->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
	 //*************************************************************************
   USART1->CR1 |= USART_CR1_UE;  // включаем USART
	 // настраиваем приорететы прирываний
	 NVIC_SetPriority(DMA1_Channel4_5_IRQn,15);
	 NVIC_SetPriority(USART1_IRQn,15);
   NVIC_EnableIRQ(DMA1_Channel4_5_IRQn); // разрешаем прерывание по ДМА
   NVIC_EnableIRQ(USART1_IRQn); // разрешаем прерывание по USART
}

// обработчик прерывания от USART
void USART1_IRQHandler()
{
  if(USART1->ISR&USART_ISR_IDLE)   // Шина свободна 
  {
    USART1->ICR|=USART_ICR_IDLECF; // сбрасываем флаг свободной шины
    USART1->CR1 &= ~USART_CR1_UE;  // отключаем USART
    USART1->CR1 &= ~USART_CR1_IDLEIE; //выключаем прерывание по свободной шине
    nRX=(nRX+1)&0x01; //вычисляем буфер для приема ДМА
    DMA1_Channel5->CPAR=(uint32_t)&USART1->RDR; //настраиваем буфер ДМА от куда пересылать
    DMA1_Channel5->CNDTR=8; // количество байт
    DMA1_Channel5->CMAR=(uint32_t)&RXbuf[nRX][0]; // приемный буфер 
    DMA1_Channel5->CCR=0; // сбрасываем регист
    DMA1_Channel5->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE; // настраиваем регистр 
    DMA1_Channel5->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
    USART1->CR1 |= USART_CR1_UE; // включаем USART
  }
  
}
// обработчик прерывания от ДМА
void DMA1_Channel4_5_IRQHandler()
{
  if (DMA1->ISR&DMA_ISR_TCIF5) // буфер заполнен
  {
    DMA1->IFCR|=DMA_IFCR_CTEIF5; // сбрасываем флаг
    DMA1_Channel5->CCR&=~DMA_CCR_EN; // выключаем канал
    USART1->CR1 &= ~USART_CR1_UE;  //выключаем USART
    USART1->CR1 |= USART_CR1_IDLEIE; // включаем режим проверки шины
    USART1->CR1 |= USART_CR1_UE; //включаем USART
    nRXpr=nRX; // устанавливаем указатель на буфер
    fRXpr=1; // устанавливаем флаг принятых данных
  }
  if (DMA1->ISR&DMA_ISR_GIF5) DMA1->IFCR|=DMA_IFCR_CGIF5; 
  //***********************************************************************
  //
  // обработчик канала для передачи
  //
  //
  //
  //***********************************************************************
  if (DMA1->ISR&DMA_ISR_TCIF4) // передача завершена по ДМА
  {
    DMA1->IFCR|=DMA_IFCR_CTEIF4; // сбрасываем бит окончания передачи
		ftx=0; // пакет передан сбрасываем флаг
  }
  if (DMA1->ISR&DMA_ISR_GIF4) 
  {
    DMA1->IFCR|=DMA_IFCR_CGIF4; 
  }
}