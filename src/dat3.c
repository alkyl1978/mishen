#include "stm32f0xx.h" 
#include "dat3.h"
#include "def_struct.h"
#include "syscal.h"

volatile int16_t sec_tim14; // счетчик секунд
dat_def dat; // структура датчика

void dat0_init()
{
	 GPIOA->MODER&=~GPIO_MODER_MODER4;
	 GPIOA->MODER|=GPIO_MODER_MODER4_1; 
	 GPIOA->AFR[0]|=(4<<(4*4));
	 // timer
	 RCC->APB1ENR|=RCC_APB1ENR_TIM14EN;
	 TIM14->PSC=48000;
	 TIM14->ARR=1000;
	 TIM14->CCMR1|= TIM_CCMR1_CC1S_0;	 
	 TIM14->CCER|= TIM_CCER_CC1E|TIM_CCER_CC1P;	 
	 TIM14->DIER |= TIM_DIER_CC1IE|TIM_DIER_UIE; 
	 TIM14->CR1|=TIM_CR1_CEN;
	 NVIC_SetPriority(TIM14_IRQn,10);
	 NVIC_EnableIRQ(TIM14_IRQn);
}

void TIM14_IRQHandler()
{
	int16_t temp;
	if(TIM14->SR&TIM_SR_CC1IF)
	{
			temp =TIM14->CCR1;
			dat.dat.ms=temp;
			dat.dat.sec=sec_tim14;
			dat.dat_per=0x39;                // сработал 0 датчик
		  dat_disable();                // выключаем датчики
			dat.flag=1;                   // устанавливаем флаг сработки датчика
			return;
	}	
	if(TIM14->SR&TIM_SR_UIF)
	{
		TIM14->SR &= ~TIM_SR_UIF;
		if(dat.enb_dat) sec_tim14++; // считаем счетчик секунд
		else sec_tim14=0;
		return;
	}	
}
	