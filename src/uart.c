#include "stm32f0xx.h" 
#include "uart.h"
#include "nrf24l01.h"

volatile unsigned char RXbuf[2][8]; 
volatile unsigned char nRXpr;       
volatile unsigned char nRX;               
volatile unsigned char fRXpr;      
volatile unsigned char TXbuf[7];   

extern volatile int8_t ftx;    // ���� ��������� �������� ������
extern RF_TypeDef RF;


void Uart_init(void) 
{
   nRXpr=0; 
   RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_DMA1EN; // �������� ������������ DMA GPIOA
   RCC->APB2ENR|=RCC_APB2ENR_USART1EN; // �������� ������������ USART
   GPIOA->MODER &= ~(GPIO_MODER_MODER9 |GPIO_MODER_MODER10); // ���������� ��������� ������
   GPIOA->MODER |=  GPIO_MODER_MODER10_1 |GPIO_MODER_MODER9_1;  // ����������� ����� ������
   GPIOA->AFR[1]|= (1<<(4*1)) |(1<<(4*2)); // �������� �������������� �������
   USART1->BRR =(APBCLK+BAUDRATE/2)/BAUDRATE;  // ��������� �������
	 // ������������� USART
	 USART1->CR1 |= USART_CR1_RE;
	 USART1->CR1 |= USART_CR1_TE;	 
	 USART1->CR1 |= USART_CR1_IDLEIE;  // 
   USART1->CR3|=USART_CR3_DMAR|USART_CR3_DMAT;  
	 //**************************************************************************
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // �������� ������
	// ������� DMA ������
	 SYSCFG->CFGR1|=SYSCFG_CFGR1_USART1TX_DMA_RMP | SYSCFG_CFGR1_USART1RX_DMA_RMP;
	 //����������� ��� ��� �������� ������
	 DMA1_Channel4->CPAR=(uint32_t)&USART1->TDR; // ����������� ����� ��� �� ���� ����������
	 DMA1_Channel4->CNDTR=7; // ���������� ����
	 DMA1_Channel4->CMAR=(uint32_t)&TXbuf[0]; // ���������� ����� 
	 DMA1_Channel4->CCR=0; // ���������� ������
	 DMA1_Channel4->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE|DMA_CCR_DIR; // ����������� ������� 
	 DMA1_Channel4->CCR|=DMA_CCR_EN;//��������� ����� ���
	 //*************************************************************************
   USART1->CR1 |= USART_CR1_UE;  // �������� USART
	 // ����������� ���������� ����������
	 NVIC_SetPriority(DMA1_Channel4_5_IRQn,15);
	 NVIC_SetPriority(USART1_IRQn,15);
   NVIC_EnableIRQ(DMA1_Channel4_5_IRQn); // ��������� ���������� �� ���
   NVIC_EnableIRQ(USART1_IRQn); // ��������� ���������� �� USART
}

// ���������� ���������� �� USART
void USART1_IRQHandler()
{
  if(USART1->ISR&USART_ISR_IDLE)   // ���� �������� 
  {
    USART1->ICR|=USART_ICR_IDLECF; // ���������� ���� ��������� ����
    USART1->CR1 &= ~USART_CR1_UE;  // ��������� USART
    USART1->CR1 &= ~USART_CR1_IDLEIE; //��������� ���������� �� ��������� ����
    nRX=(nRX+1)&0x01; //��������� ����� ��� ������ ���
    DMA1_Channel5->CPAR=(uint32_t)&USART1->RDR; //����������� ����� ��� �� ���� ����������
    DMA1_Channel5->CNDTR=8; // ���������� ����
    DMA1_Channel5->CMAR=(uint32_t)&RXbuf[nRX][0]; // �������� ����� 
    DMA1_Channel5->CCR=0; // ���������� ������
    DMA1_Channel5->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE; // ����������� ������� 
    DMA1_Channel5->CCR|=DMA_CCR_EN;//��������� ����� ���
    USART1->CR1 |= USART_CR1_UE; // �������� USART
  }
  
}
// ���������� ���������� �� ���
void DMA1_Channel4_5_IRQHandler()
{
  if (DMA1->ISR&DMA_ISR_TCIF5) // ����� ��������
  {
    DMA1->IFCR|=DMA_IFCR_CTEIF5; // ���������� ����
    DMA1_Channel5->CCR&=~DMA_CCR_EN; // ��������� �����
    USART1->CR1 &= ~USART_CR1_UE;  //��������� USART
    USART1->CR1 |= USART_CR1_IDLEIE; // �������� ����� �������� ����
    USART1->CR1 |= USART_CR1_UE; //�������� USART
    nRXpr=nRX; // ������������� ��������� �� �����
    fRXpr=1; // ������������� ���� �������� ������
  }
  if (DMA1->ISR&DMA_ISR_GIF5) DMA1->IFCR|=DMA_IFCR_CGIF5; 
  //***********************************************************************
  //
  // ���������� ������ ��� ��������
  //
  //
  //
  //***********************************************************************
  if (DMA1->ISR&DMA_ISR_TCIF4) // �������� ��������� �� ���
  {
    DMA1->IFCR|=DMA_IFCR_CTEIF4; // ���������� ��� ��������� ��������
		ftx=0; // ����� ������� ���������� ����
  }
  if (DMA1->ISR&DMA_ISR_GIF4) 
  {
    DMA1->IFCR|=DMA_IFCR_CGIF4; 
  }
}