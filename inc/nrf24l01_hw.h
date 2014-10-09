#ifndef __NRF24L01HW_H_
#define __NRF24L01HW_H_

#include "stm32f0xx.h" 

#define SPI1_DR_8bit          (*(__IO uint8_t *)((uint32_t)&(SPI1->DR))) 

#define CSN_ON GPIOA->BSRR|=GPIO_BSRR_BS_12
#define CSN_OFF  GPIOA->BSRR|=GPIO_BSRR_BR_12

#define CE_ON GPIOA->BSRR|=GPIO_BSRR_BS_11
#define CE_OFF  GPIOA->BSRR|=GPIO_BSRR_BR_11


void Spi_send (uint32_t * data, uint8_t count);

#endif
