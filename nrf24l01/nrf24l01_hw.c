#include "stm32f0xx.h" 
#include "nrf24l01_hw.h"
#include "nrf24l01.h"

RF_TypeDef RF;
//******************************************************************************************************
// начальная настройка SPI и портов
//******************************************************************************************************
void nrf24l01_init(void)
{	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // включаем тактирование GPIOA
	GPIOA->MODER &= ~(GPIO_MODER_MODER5 |GPIO_MODER_MODER6|GPIO_MODER_MODER7|GPIO_MODER_MODER8|GPIO_MODER_MODER11|GPIO_MODER_MODER12); // сбрасываем настройки портов
  GPIOA->MODER |=  GPIO_MODER_MODER5_1 |GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1;  // настраиваем режим работы SPI
	// подключаем альтернативные функции
	GPIOA->AFR[0]|=(0<<(4*5)|0<<(4*6)|0<<(4*7)); // подключаем SPI к ножкам
	// настраиваем IRQ
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPDR8;
	GPIOA->PUPDR|=GPIO_PUPDR_PUPDR8_1;
	// настраиваем CE
	CE_OFF;
	GPIOA->MODER|=GPIO_MODER_MODER11_0;
	// настраиваем CSN
	CSN_ON ;  // выключаем SPI
	GPIOA->MODER|=GPIO_MODER_MODER12_0;	
	RCC->APB2ENR |=RCC_APB2ENR_SPI1EN; //включаем SPI
	SPI1->CR1 |= SPI_CR1_BR_0|
							 SPI_CR1_BR_1|
							 SPI_CR1_BR_2|
							 SPI_CR1_MSTR | 
							 SPI_CR1_SSI  |    
							 SPI_CR1_SSM; 
	SPI1->CR2  = 0x700;   //  8 bit
	SPI1->CR2 |= SPI_CR2_FRXTH|SPI_CR2_TXDMAEN |SPI_CR2_RXDMAEN;
	//****************************************************************************************************
	// настраиваем DMA
	//****************************************************************************************************
	// канал на передачу
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // включаем тактирование DMA
	DMA1_Channel3->CPAR=(uint32_t)&SPI1->DR;
	DMA1_Channel3->CMAR=(uint32_t)&RF.nrf24l01_tx; // передающий буфер 
	DMA1_Channel3->CCR=0; // сбрасываем регист
	DMA1_Channel3->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE|DMA_CCR_DIR; // настраиваем регистр
	// канал наприем**************************************************************************************
	DMA1_Channel2->CPAR=(uint32_t)&SPI1->DR; //настраиваем буфер ДМА от куда пересылать
  DMA1_Channel2->CMAR=(uint32_t)&RF.nrf24l01_rx; // приемный буфер 
  DMA1_Channel2->CCR=0; // сбрасываем регист
  DMA1_Channel2->CCR|=DMA_CCR_MINC|DMA_CCR_TCIE; // настраиваем регистр 
	//****************************************************************************************************
	SPI1->CR1|=SPI_CR1_SPE; //включаем SPI
	//****************************************************************************************************
	// настраиваем прерывание от IRQ
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[3]&=~SYSCFG_EXTICR3_EXTI8;
	EXTI->IMR|=EXTI_IMR_MR8;
	EXTI->FTSR|=EXTI_FTSR_TR8;
	//***************************************************************************************************
	// разрешаем прерывание от NRF24L01
	//***************************************************************************************************
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	//***************************************************************************************************
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn); // разрешаем прерывание по ДМА
}
//******************************************************************************************************
// передача одного байта по SPI
void Spi_send (uint32_t *data, uint8_t count)
{    
	 while(RF.SPI_TX_ENABLE);    // ожидаем окончания передачи жидание освобождения SPI
	 DMA1_Channel3->CCR&=~DMA_CCR_EN;
	 DMA1_Channel2->CCR&=~DMA_CCR_EN;
	 DMA1_Channel3->CMAR=(uint32_t)data;
	 DMA1_Channel3->CNDTR=count; 
	 DMA1_Channel2->CNDTR=count;
	 CSN_OFF;
	 DMA1_Channel3->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
	 DMA1_Channel2->CCR|=DMA_CCR_EN;//разрешаем канал ДМА	
	 RF.SPI_TX_ENABLE=1;
}


// обработчик от прерывания от NRF24L01
void EXTI4_15_IRQHandler()
{
	if(EXTI->PR & EXTI_PR_PR8)
	{
		EXTI->PR|=EXTI_PR_PR8;
		RF.NRF_IRQ=1;
		CE_OFF;
	}
}

// обработчик прерывания от ДМА SPI канала
void DMA1_Channel2_3_IRQHandler()
{
	uint8_t status;
	//***********************************************************************
	//
	// обработка окончания передачи по DMA
	//
	//************************************************************************
	if (DMA1->ISR&DMA_ISR_TCIF3) // буфер передан
  {
		DMA1->IFCR|=DMA_IFCR_CTEIF3|DMA_IFCR_CGIF3; // сбрасываем флаг	
	}
//*************************************************************************
//
// обработка окончания приема по DMA
//
//*************************************************************************
	if (DMA1->ISR&DMA_ISR_TCIF2) // буфер принят
  {
		DMA1->IFCR|=DMA_IFCR_CTEIF2|DMA_IFCR_CGIF2; // сбрасываем флаг
		CSN_ON; // выключаем SPI;
		RF.SPI_TX_ENABLE=0;
		RF.RX_BUSY=1;
	}
}


