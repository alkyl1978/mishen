#include "stm32f0xx.h" 
#include "dat2.h"
#include "def_struct.h"
#include "syscal.h"

extern dat_def dat; // структура датчика
volatile int16_t sec_tim3;

void dat4_5_6_7_init()
{
	// включаем тактирование порта
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	// настраиваем порт
	GPIOB->MODER&=~(GPIO_MODER_MODER0|GPIO_MODER_MODER1 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
	GPIOB->MODER |=(GPIO_MODER_MODER0_1 |GPIO_MODER_MODER1_1 |
									  GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 );
	GPIOB->AFR[0]|=(1<<(4*0))|(1<<(4*1))|(1<<(4*4))|(1<<(4*5));
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN ; //включаем тактирование таймера
	TIM3->PSC=48000;
	TIM3->ARR=1000;
	// dat
	 TIM3->CCMR1|= TIM_CCMR1_CC1S_0;	
	 TIM3->CCER|= TIM_CCER_CC1E|TIM_CCER_CC1P;
	// dat
	 TIM3->CCMR1|= TIM_CCMR1_CC2S_0;
	 TIM3->CCER|= TIM_CCER_CC2E|TIM_CCER_CC2P;
  // dat
	 TIM3->CCMR2|= TIM_CCMR2_CC3S_0;
	 TIM3->CCER|= TIM_CCER_CC3E|TIM_CCER_CC3P;
	// dat
	 TIM3->CCMR2|= TIM_CCMR2_CC4S_0;	
	 TIM3->CCER|= TIM_CCER_CC4E|TIM_CCER_CC4P;
	 TIM3->DIER |= TIM_DIER_CC1IE|
								 TIM_DIER_CC2IE|
								 TIM_DIER_CC3IE|
								 TIM_DIER_CC4IE|
								 TIM_DIER_UIE; 
	 TIM3->CR1|=TIM_CR1_CEN;
	 NVIC_SetPriority(TIM3_IRQn,14);
	 NVIC_EnableIRQ(TIM3_IRQn);
}


void TIM3_IRQHandler()
{
	int16_t temp;
	// ДАТЧИК 7
	if(TIM3->SR&TIM_SR_CC1IF)
	{
		temp=TIM3->CCR1;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim3;
		dat.dat_per=0x37;                // сработал 7 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 8
	if(TIM3->SR&TIM_SR_CC2IF)
	{
		temp=TIM3->CCR2;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim3;
		dat.dat_per=0x38;                // сработал 8 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 6
	if(TIM3->SR&TIM_SR_CC3IF)
	{
		temp=TIM3->CCR3;
		dat.dat.ms=temp;
		dat.dat.sec=sec_tim3;
		dat.dat_per=0x36;              // сработал 6 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 3
	if(TIM3->SR&TIM_SR_CC4IF)
	{
		temp=TIM3->CCR4;
		dat.dat.ms=temp;
		dat.dat.sec=sec_tim3;
		dat.dat_per=0x33;              // сработал 3 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	if(TIM3->SR&TIM_SR_UIF)
	{
		TIM3->SR &= ~TIM_SR_UIF;
		if(dat.enb_dat) sec_tim3++; // считаем счетчик секунд
		else sec_tim3=0;
		return;
	}
}

