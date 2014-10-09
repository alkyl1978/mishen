#include "stm32f0xx.h" 
#include "nrf24l01_hw.h"
#include "nrf24l01.h"

RF_TypeDef RF;
//******************************************************************************************************
// ��������� ��������� SPI � ������
//******************************************************************************************************
void nrf24l01_init(void)
{	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // �������� ������������ GPIOA
	GPIOA->MODER &= ~(GPIO_MODER_MODER5 |GPIO_MODER_MODER6|GPIO_MODER_MODER7|GPIO_MODER_MODER8|GPIO_MODER_MODER11|GPIO_MODER_MODER12); // ���������� ��������� ������
  GPIOA->MODER |=  GPIO_MODER_MODER5_1 |GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1;  // ����������� ����� ������ SPI
	// ���������� �������������� �������
	GPIOA->AFR[0]|=(0<<(4*5)|0<<(4*6)|0<<(4*7)); // ���������� SPI � ������
	// ����������� IRQ
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPDR8;
	GPIOA->PUPDR|=GPIO_PUPDR_PUPDR8_1;
	// ����������� CE
	CE_OFF;
	GPIOA->MODER|=GPIO_MODER_MODER11_0;
	// ����������� CSN
	CSN_ON ;  // ��������� SPI
	GPIOA->MODER|=GPIO_MODER_MODER12_0;	
	RCC->APB2ENR |=RCC_APB2ENR_SPI1EN; //�������� SPI
	SPI1->CR1 |= SPI_CR1_BR_0|
							 SPI_CR1_BR_1|
							 SPI_CR1_BR_2|
							 SPI_CR1_MSTR | 
							 SPI_CR1_SSI  |    
							 SPI_CR1_SSM; 
	SPI1->CR2  = 0x700;   //  8 bit
	SPI1->CR2 |= SPI_CR2_FRXTH|SPI_CR2_TXDMAEN |SPI_CR2_RXDMAEN;
	//****************************************************************************************************
	// ����������� DMA
	//****************************************************************************************************
	// ����� �� ��������
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // �������� ������������ DMA
	DMA1_Channel3->CPAR=(uint32_t)&SPI1->DR;
	DMA1_Channel3->CMAR=(uint32_t)&RF.nrf24l01_tx; // ���������� ����� 
	DMA1_Channel3->CCR=0; // ���������� ������
	DMA1_Channel3->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE|DMA_CCR_DIR; // ����������� �������
	// ����� �������**************************************************************************************
	DMA1_Channel2->CPAR=(uint32_t)&SPI1->DR; //����������� ����� ��� �� ���� ����������
  DMA1_Channel2->CMAR=(uint32_t)&RF.nrf24l01_rx; // �������� ����� 
  DMA1_Channel2->CCR=0; // ���������� ������
  DMA1_Channel2->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE; // ����������� ������� 
	//****************************************************************************************************
	SPI1->CR1|=SPI_CR1_SPE; //�������� SPI
	//****************************************************************************************************
	// ����������� ���������� �� IRQ
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[3]&=~SYSCFG_EXTICR3_EXTI8;
	EXTI->IMR|=EXTI_IMR_MR8;
	EXTI->FTSR|=EXTI_FTSR_TR8;
	//***************************************************************************************************
	// ��������� ���������� �� NRF24L01
	//***************************************************************************************************
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	//***************************************************************************************************
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn); // ��������� ���������� �� ���
}
//******************************************************************************************************
// �������� ������ ����� �� SPI
void Spi_send (uint32_t *data, uint8_t count)
{    
	 while(RF.SPI_TX_ENABLE);    // ������� ��������� �������� ������� ������������ SPI
	 DMA1_Channel3->CCR&=~DMA_CCR_EN;
	 DMA1_Channel2->CCR&=~DMA_CCR_EN;
	 DMA1_Channel3->CMAR=(uint32_t)data;
	 DMA1_Channel3->CNDTR=count; 
	 DMA1_Channel2->CNDTR=count;
	 CSN_OFF;
	 DMA1_Channel3->CCR|=DMA_CCR_EN;//��������� ����� ���
	 DMA1_Channel2->CCR|=DMA_CCR_EN;//��������� ����� ���	
	 RF.SPI_TX_ENABLE=1;
}


// ���������� �� ���������� �� NRF24L01
void EXTI4_15_IRQHandler()
{
	if(EXTI->PR & EXTI_PR_PR8)
	{
		EXTI->PR|=EXTI_PR_PR8;
		RF.NRF_IRQ=1;
		CE_OFF;
	}
}

// ���������� ���������� �� ��� SPI ������
void DMA1_Channel2_3_IRQHandler()
{
	uint8_t status;
	//***********************************************************************
	//
	// ��������� ��������� �������� �� DMA
	//
	//************************************************************************
	if (DMA1->ISR&DMA_ISR_TCIF3) // ����� �������
  {
		DMA1->IFCR|=DMA_IFCR_CTEIF3|DMA_IFCR_CGIF3; // ���������� ����	
	}
//*************************************************************************
//
// ��������� ��������� ������ �� DMA
//
//*************************************************************************
	if (DMA1->ISR&DMA_ISR_TCIF2) // ����� ������
  {
		DMA1->IFCR|=DMA_IFCR_CTEIF2|DMA_IFCR_CGIF2; // ���������� ����
		CSN_ON; // ��������� SPI;
		RF.SPI_TX_ENABLE=0;
		RF.RX_BUSY=1;
	}
}


