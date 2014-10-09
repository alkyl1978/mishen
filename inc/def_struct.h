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
	tim_def dat; // ����� ���� �������
	volatile uint16_t flag;              // �������� ������
	volatile uint16_t dat_per;           // ������ ����������� ������
	volatile uint16_t temp_game;         //���� ����
	volatile uint16_t pause_game;        //����� ����
	volatile uint16_t time_d;            //����� ������������
	volatile uint8_t cmd;                //������� ����� ������
	volatile uint16_t time_p;            // �������� ����� �����
	volatile uint16_t time_t;            //�������� ���� ����
	volatile uint16_t reg_rab;           // ������� ����� ������
	volatile uint16_t  enb_dat;       // ������� ���� ���������
	volatile uint16_t  enb_time_pause;  // �������� �����
	volatile uint16_t  enb_time_temp;    // �������� ����.
	volatile uint16_t  napr_poleta;       // ����������� ������ ������
	volatile uint16_t enb_rak;            // ���������� ������� �������
	volatile uint16_t count_A;            // ������� ������ �� ���� �
	volatile uint16_t count_B;            // ������� ������ �� ���� �
	volatile uint16_t count_R;            // ������� ������ �� ������� 0
} dat_def;
#endif