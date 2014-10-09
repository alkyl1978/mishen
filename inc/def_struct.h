#ifndef DEF_H
#define DEF_H

#include "stm32f0xx.h" 

typedef struct
{
	uint16_t ms;
	uint16_t sec;
} tim_def;

typedef struct
{
	tim_def dat; // время сраб датчика
	volatile uint16_t flag;              // сработал датчик
	volatile uint16_t dat_per;           // первый сработавший датчик
	volatile uint16_t temp_game;         //темп игры
	volatile uint16_t pause_game;        //пауза игры
	volatile uint16_t time_d;            //время антидребезга
	volatile uint8_t cmd;                //текущий режим работы
	volatile uint16_t time_p;            // заданное время паузы
	volatile uint16_t time_t;            //заданное темп игры
	volatile uint16_t reg_rab;           // текущий режим работы
	volatile uint16_t  enb_dat;       // датчики поля разрешено
	volatile uint16_t  enb_time_pause;  // работает пауза
	volatile uint16_t  enb_time_temp;    // работает темп.
	volatile uint16_t  napr_poleta;       // направление полета шарика
	volatile uint16_t enb_rak;            // разрешения работки ракетки
	volatile uint16_t count_A;            // счетчик ударов по полю А
	volatile uint16_t count_B;            // счетчик ударов по полю Б
	volatile uint16_t count_R;            // счетчик ударов по ракетке 0
} dat_def;
#endif