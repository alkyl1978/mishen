#include "stm32f0xx.h" 
#include "dat0.h"
#include "def_struct.h"
#include "syscal.h"

int8_t flag;
extern dat_def dat; // структура датчика
volatile int16_t sec_tim2;

void dat1_2_3_init()
{
	 GPIOA->MODER&=~(GPIO_MODER_MODER1|GPIO_MODER_MODER2|GPIO_MODER_MODER3);
	 GPIOA->MODER|=GPIO_MODER_MODER1_1|GPIO_MODER_MODER2_1|GPIO_MODER_MODER3_1;
	 GPIOA->AFR[0]|=(2<<(4*1))|(2<<(4*2))|(2<<(4*3));
	// timer
	 RCC->APB1ENR|=RCC_APB1ENR_TIM2EN ; //set clock;
	 TIM2->PSC=48000;
	 TIM2->ARR=1000;
	// dat1
	 TIM2->CCMR1|= TIM_CCMR1_CC2S_0;	
	 TIM2->CCER|= TIM_CCER_CC2E|TIM_CCER_CC2P;
	// dat2
	 TIM2->CCMR2|= TIM_CCMR2_CC3S_0;
	 TIM2->CCER|= TIM_CCER_CC3E|TIM_CCER_CC3P;
  // dat3
	 TIM2->CCMR2|= TIM_CCMR2_CC4S_0;
	 TIM2->CCER|= TIM_CCER_CC4E|TIM_CCER_CC4P;
	
	 TIM2->DIER |= TIM_DIER_CC2IE|
								 TIM_DIER_CC3IE|
								 TIM_DIER_CC4IE|
								 TIM_DIER_UIE; 
	 TIM2->CR1|=TIM_CR1_CEN;
	 NVIC_SetPriority(TIM2_IRQn,14);
	 NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler()
{
	int16_t temp;
	// датчик 5
	if(TIM2->SR&TIM_SR_CC2IF)
	{
		temp=TIM2->CCR2;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim2;
		dat.dat_per=0x35;             // сработал 5 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 0
	if(TIM2->SR&TIM_SR_CC3IF)
	{
		temp=TIM2->CCR3;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim2;
		dat.dat_per=0x30;                 // сработал 0 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	// датчик 2
	if(TIM2->SR&TIM_SR_CC4IF)
	{
		temp=TIM2->CCR4;
		dat.dat.ms=temp;	
		dat.dat.sec=sec_tim2;
		dat.dat_per=0x32;              // сработал 2 датчик
		dat_disable();                // выключаем датчики
		dat.flag=1;                   // устанавливаем флаг сработки датчика
		return;
	}
	if(TIM2->SR&TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;
		if(dat.enb_dat) sec_tim2++; // считаем счетчик секунд
		else sec_tim2=0;
		return;
	}
}

	 