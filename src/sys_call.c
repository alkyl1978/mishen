#include "stm32f0xx.h"
#include "syscal.h"
#include "def_struct.h"


extern dat_def dat; // структура датчика

// функция блокирования датчиков
void dat_disable()
{
	//отключаем прерывания по захвату.
	TIM2->CCER &= ~(TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E);
	TIM14->CCER &= ~TIM_CCER_CC1E;
	TIM3->CCER &= ~(TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E);
	TIM15->CCER &= ~(TIM_CCER_CC1E|TIM_CCER_CC2E);
	dat.enb_dat=0;
	dat.enb_rak=0;
	dat.flag=0;
	TIM14->EGR|=TIM_EGR_UG;
	TIM2->EGR|=TIM_EGR_UG;
	TIM3->EGR|=TIM_EGR_UG;
	TIM15->EGR|=TIM_EGR_UG;
}

// разрешаем датчики
void dat_enable()
{
	dat.enb_dat=1;
	dat.enb_rak=1;
	TIM14->CCER |= TIM_CCER_CC1E; // разрешаем прерывания по захвату.
	TIM2->CCER |= (TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E);
	TIM3->CCER |= (TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E);
	TIM15->CCER |= (TIM_CCER_CC1E|TIM_CCER_CC2E);	
}