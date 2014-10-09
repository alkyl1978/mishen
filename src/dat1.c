#include "stm32f0xx.h" 
#include "dat1.h"
#include "def_struct.h"
#include "syscal.h"

extern dat_def dat; // структура датчика
volatile int16_t sec_tim15;

void dat8_9_init()
{
	// включаем тактирование порта
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	// настраиваем порт
	GPIOB->MODER&=~(GPIO_MODER_MODER14|GPIO_MODER_MODER15);
	GPIOB->MODER |=(GPIO_MODER_MODER14_1 |GPIO_MODER_MODER15_1 );
	GPIOB->AFR[1]|=(1<<(4*6))|(1<<(4*7));
	RCC->APB2ENR|=RCC_APB2ENR_TIM15EN ; //включаем тактирование таймера
	TIM15->PSC=48000;
	TIM15->ARR=1000;
	// dat
	 TIM15->CCMR1|= TIM_CCMR1_CC1S_0;	
	 TIM15->CCER|= TIM_CCER_CC1E|TIM_CCER_CC1P;
	// dat
	 TIM15->CCMR1|= TIM_CCMR1_CC2S_0;	
	 TIM15->CCER|= TIM_CCER_CC2E|TIM_CCER_CC2P;
	 TIM15->DIER |= TIM_DIER_CC1IE|
									TIM_DIER_CC2IE|
									TIM_DIER_UIE; 
	 TIM15->CR1|=TIM_CR1_CEN;
	 NVIC_SetPriority(TIM15_IRQn,14);
	 NVIC_EnableIRQ(TIM15_IRQn);
}

void TIM15_IRQHandler()
{
	int16_t temp;
	// ДАТЧИК 1
	if(TIM15->SR&TIM_SR_CC1IF)
	{
		temp=TIM15->CCR1;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim15;
		dat.dat_per=0x31;              // сработал 1 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 4
	if(TIM15->SR&TIM_SR_CC2IF)
	{
		temp=TIM15->CCR2;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim15;
		dat.dat_per=0x34;         // сработал 4 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	if(TIM15->SR&TIM_SR_UIF)
	{
		TIM15->SR &= ~TIM_SR_UIF;
		if(dat.enb_dat) sec_tim15++; // считаем счетчик секунд
		else sec_tim15=0;
		return;
	}
}