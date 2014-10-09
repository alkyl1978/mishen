#include "stm32f0xx.h"
#include "def_struct.h"
extern dat_def dat; // структура датчика
extern volatile int16_t sec_tim2; // счетчик секунд
extern volatile int16_t sec_tim3;
extern volatile int16_t sec_tim15;
extern volatile int16_t sec_tim14;
volatile uint8_t time_nrf;

void SysTick_Handler(void) 
	{		
		  if(time_nrf) time_nrf--;
			// обработчики паузы и темпа в игре.
			if(dat.enb_time_pause)
			{
				if(dat.pause_game!=0) dat.pause_game--;
				else
				{
					dat.enb_time_pause=0;
					//сбрасываем счетчики
					sec_tim2=0;
					sec_tim3=0;
					sec_tim15=0;
					sec_tim14=0;
					TIM14->EGR|=TIM_EGR_UG;
					TIM2->EGR|=TIM_EGR_UG;
					TIM3->EGR|=TIM_EGR_UG;
					TIM15->EGR|=TIM_EGR_UG;
				}
			}
			if(dat.enb_time_temp)
			{
				if(dat.temp_game!=0) dat.temp_game--;
				else
				{
					dat.enb_time_temp=0;
					sec_tim2=0;
					sec_tim3=0;
					sec_tim15=0;
					sec_tim14=0;
					TIM14->EGR|=TIM_EGR_UG;
					TIM2->EGR|=TIM_EGR_UG;
					TIM3->EGR|=TIM_EGR_UG;
					TIM15->EGR|=TIM_EGR_UG;
				}
			}
	}
	