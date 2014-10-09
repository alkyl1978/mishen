#include "stm32f0xx.h" 
#include "uart.h"
#include "dat0.h"
#include "dat1.h"
#include "dat2.h"
#include "dat3.h"
#include "def_struct.h"
#include "syscal.h"
#include "nrf24l01.h"
#include "nrf24l01_hw.h"
#include "nrf24l01_Wizard.h"

extern RF_TypeDef RF;
extern dat_def dat; // структура датчика
int16_t crc16_modbus(unsigned char *buf,unsigned char len);
extern volatile unsigned char RXbuf[2][8]; 
extern volatile unsigned char nRXpr;               
extern volatile unsigned char fRXpr;   
extern volatile unsigned char TXbuf[8];  //передающий буфер
extern volatile int16_t sec;
volatile uint8_t ftx; // флаг указателя передачи пакета
extern volatile uint8_t time_nrf;
//*****************************************************************************
uint16_t crc;
uint32_t time;
tim_def tim0;
uint8_t temp;
//***************************************************************************
int main(void)
   { 	
	 uint8_t status;
	 SysTick_Config(SystemCoreClock /1000);  //счетчик милисекунд
	 Uart_init();
	 dat0_init(); 
	 dat1_2_3_init();
	 dat4_5_6_7_init();
	 dat8_9_init();
	 nrf24l01_init();
	 RF_Init(); // начальная настройка NRF24L01
	 CE_ON;
	 time_nrf=200;
	 while(time_nrf){}; // пауза
   while(1)
    {		
			// есть прерывание обрабатываем
			if(RF.NRF_IRQ || !(GPIOA->IDR&(1<<8)))
			{
				status=RF_Read_Cmd(STATUS_REG); // читаем статус регистр
				while(status!=0x0e)
				{
					if(status&RF_FIFO_FULL)
					{
						RF_Flush(NRF_CMD_FLUSH_RX);// буфер переполнен.
						RF.NRF_RX=2; // устанавливаем флаг ошибки передачи данных.
						status=RF_Read_Cmd(STATUS_REG); // читаем статус регистр
					}
					if(status&RF_MAX_RT_IRQ_CLEAR)
					{
						RF_Send_Cmd(STATUS_REG,RF_MAX_RT_IRQ_CLEAR);
						RF.NRF_TX=2; // устанавливаем флаг ошибки передачи данных.
					}					
					if(status&RF_TX_DS_IRQ_CLEAR) 
					{
						RF.NRF_TX =1; // устанавливаем флаг данные переданы.
						RF.NRF_RX =0;
						RF_Send_Cmd(STATUS_REG,RF_TX_DS_IRQ_CLEAR);
					}
					if(status&RF_RX_DR_IRQ_CLEAR)
					{
						dat_disable();                // выключаем датчики
						RF.RX_BUSY=0;
						RF.Pid=status>>1;  // сохраняем принятый пид.
						RF_ReadPayload(1); // считываем данные
						while(!RF.RX_BUSY);  // ожидаем окончания приема.
						RF.NRF_RX=1;       // данные приняты
						dat.flag=1;
						dat.dat_per=0x0a;
						RF.RX_BUSY=0;
						RF_Send_Cmd(STATUS_REG,RF_RX_DR_IRQ_CLEAR);
					}
					status=RF_Read_Cmd(STATUS_REG); // читаем статус регистр
					if(status!=0x0e)
					{
						dat_disable();                // выключаем датчики
						RF.RX_BUSY=0;
						RF.Pid=status>>1;  // сохраняем принятый пид.
						RF_ReadPayload(1); // считываем данные
						while(!RF.RX_BUSY);  // ожидаем окончания приема.
						RF.NRF_RX=1;       // данные приняты
						dat.flag=1;
						dat.dat_per=0x0a;
						RF.RX_BUSY=0;
						status=RF_Read_Cmd(STATUS_REG); // читаем статус регистр
					}
				}
					RF.NRF_IRQ=0;
					CE_ON; //включаем приемопередатчик
			}
			if(DMA1_Channel4->CNDTR==0 && ftx)
			{
				DMA1_Channel4->CCR&=~DMA_CCR_EN;
				DMA1_Channel4->CNDTR=7; // количество байт
				// расчитываем CRC
				crc=crc16_modbus((unsigned char *)&TXbuf[0],4);
				TXbuf[5]=(crc>>8)&0xff;
				TXbuf[6]=crc&0xff;
				DMA1_Channel4->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
			}
			// проверка флага приема
			if(fRXpr)
			{
				crc=crc16_modbus((unsigned char *)&RXbuf[nRXpr][0],5);
				if(((crc>>8)&0xff)==RXbuf[nRXpr][6] && (crc&0xff)==RXbuf[nRXpr][7])
				{
					// сохраняем принятые настройки
					switch (RXbuf[nRXpr][0])
					{
						case 0xCC: // приняли команду
					{
						dat.cmd=RXbuf[nRXpr][2];
						dat.time_d=RXbuf[nRXpr][3]*8; // время антидребезга
						dat.time_p=RXbuf[nRXpr][5]*16; // время паузы
						dat.time_t=RXbuf[nRXpr][4]*8;  // темп игры
						TXbuf[0]=dat.cmd; // устанавливаем новый режим работы
						TXbuf[1]=0x03; // количество байт
						//запрещаем прерывания по захвату.
						dat_disable();
						// настраиваем в зависимости от команды
						//***********************************************************************
						switch(dat.cmd)
						{
							//************************************************************************
							//
							//  работа с роботом активной ракеткой
							//
							//************************************************************************
							case 0xAA:
							{
								//  обнуляем счетчики ударов по полям
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // разрешаем датчики
								dat.reg_rab=0;  // устанавливаем текущий режим работы.
								break;
							}
							case 0x55:
							{
								//**********************************************************************
								//
								//
								// первоначальная настройка подач.
								//
								//
								//**********************************************************************
								//  обнуляем счетчики ударов по полям
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // разрешаем датчики
								dat.reg_rab=0;  // устанавливаем текущий режим работы.
								break;
							}
							case 0x99:
							{
								//*******************************************************************
								// первоначальная настройка при работе с пасивными ракетками
								//*******************************************************************
								//  обнуляем счетчики ударов по полям
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // разрешаем датчики
								dat.reg_rab=0;  // устанавливаем текущий режим работы.
								break;
							}
							case 0x33:
							{
								//*******************************************************************
								// первоначальная настройка при тесте датчиков и ракеток
								//********************************************************************
								//  обнуляем счетчики ударов по полям
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // разрешаем датчики
								dat.reg_rab=0;  // устанавливаем текущий режим работы.
								break;
							}
						}
						//***********************************************************************
						break;
					}
						case 0x99: 	// ошибка повторить
						{
							ftx=1; 		// повторно передаем
							break;
						}
						case 0x33: 	// индефикатор ошибка приема
						{
							break;
						}
					}
				}
				fRXpr=0; // сбрасываем флаг приема данные обработаны
			}
/// ********************************************************************************************************
// основной цикл обработки
//**********************************************************************************************************
			switch(dat.cmd)
			{
	//****************************************************************************************************
				//
				//
				// обработчик работа с ракеткой 
//********************************************************************************************************
				case 0xAA:
				{
					switch(dat.reg_rab)
					{
						case 0:
						{
							if(dat.flag==1 && ftx!=1) // сработал датчик стола
							{
								if(dat.dat_per==0) // сработал нулевой датчик
								{
									if(dat.count_A==0) /// сработал датчик А
									{
										// передаем сработавший датчик
										TXbuf[2]=dat.dat_per;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1;
									}
									else
									{
										if(dat.count_R!=0) // был уже удар по ракетки 
										{
											// передаем номер датчика
											TXbuf[2]=dat.dat_per;
											time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
											TXbuf[3]=(time>>8)&0xff;
											TXbuf[4]=time&0xff;
											ftx=1;
										}
									}
									dat.count_A++;
								}
								else  // сработали другие датчики и ракетка
								{
									if(dat.dat_per<0x0a) // сработало поле Б
									{
										if(dat.count_A!=0) // есть шарики в полете
										{
											// передаем номер датчика
											TXbuf[2]=dat.dat_per;
											time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
											TXbuf[3]=(time>>8)&0xff;
											TXbuf[4]=time&0xff;
											ftx=1;
											// убираем отработавший шарик.
											dat.count_A--; 
											dat.count_R--;
										}
										if(dat.count_R>2) // есть улетевшие шарики 
										{
											dat.count_R=1; // убираем эти шарики
										}
									}
									else  // сработала ракетка
									{
										if(dat.count_A!=0)
										{
											// передаем номер ракетки
											TXbuf[2]=0x21;
											time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
											TXbuf[3]=(time>>8)&0xff;
											TXbuf[4]=time&0xff;
											ftx=1;
										}											
										dat.count_R++;
										if(dat.count_A!=dat.count_R)
										{
											dat.count_A=dat.count_R;
										}
									}
								}
								dat.reg_rab=1;
								// устанавливаем паузу.
								dat.enb_time_pause=0;
								dat.pause_game=dat.time_p;
								dat.enb_time_pause=1;
								dat_disable(); // выключаем датчики
							}
							break;
						}
						case 1: // обработчик пауз.
						{
							if(!dat.enb_time_pause) // ожидаем окончания паузы
							{
								dat_enable(); // разрешаем все датчики и ракетки
							}
							break;
						}
					}
					break;
				}
//***************************************************************************************************
// работает режим теста стола
//
//
//***************************************************************************************************
				case 0x33:
				{
					switch(dat.reg_rab)
					{
						case 0:
						{
							//проверяем есть ли сработавшие датчики.
							if(ftx!=1 && dat.flag==1)
							{
								// передаем сраб датчик
								if(dat.dat_per==0x0A) TXbuf[2]=0x21;
								else TXbuf[2]=dat.dat_per;
								TXbuf[3]=dat.dat.sec;
								TXbuf[4]=dat.dat.ms;
								ftx=1; // устанавливаем флаг передачи.
								dat.reg_rab=1;
								// устанавливаем паузу.
								dat.enb_time_pause=0;
								dat.pause_game=dat.time_p;
								dat.enb_time_pause=1;
								dat_disable(); // выключаем датчики
							}
							break;
						}
						case 1:
						{
							if(!dat.enb_time_pause) // ожидаем окончания паузы
								{
									// пауза вышла включаем все датчики
									dat.reg_rab=0;
									// разрешаем датчики.
									dat_enable();
								}
							break;
						}
					}
					break;
				}
//******************************************************************************************************
//основной алгоритм работы мишени при 2 игрока пассивно
//
//******************************************************************************************************
				case 0x99:
				{
						/// основной алгоритм работы мишени при 2 игрока пассивно
						switch(dat.reg_rab)
						{
							case 0x00:
							{
								if(dat.flag==1) // есть сработавший датчик
								{
									if(dat.dat_per==0) // сработал первый датчик
									{
										dat.napr_poleta=1;   // устанавливаем флаг направления полета
										dat.count_A++;       // увеличиваем счетчик поля А
									}
									else
									{
										dat.napr_poleta=0;   //устанавливаем флаг полета
										dat.count_B++;       // счетчик поля Б
									}
									dat.reg_rab=1; // переходим на второй режим	
									// устанавливаем паузу.
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p;
									dat.enb_time_pause=1;
									// запрещаем все поля датчиков
									dat_disable();
							  }
							  break;
							}
							case 0x01:
							{
								if(!dat.enb_time_pause) // ожидаем окончания паузы
								{
									// пауза вышла включаем все датчики
									if(dat.napr_poleta) dat.reg_rab=2;
									else dat.reg_rab=0;
									// разрешаем датчики.
									dat_enable();
								}
								break;
							}
							case 0x02:
							{
								if(!ftx && dat.flag==1) // сработал датчик
								{
									if(dat.dat_per==0) // сработал поле А
									{
										dat.reg_rab=1;
										dat.napr_poleta=1;
										dat.count_A++;
									}
									if(dat.dat_per!=0) // сработали другие датчики
									{
										dat.count_B++;
										// передаем сработавший датчик
										TXbuf[2]=dat.dat_per;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1;
										dat.reg_rab=3;		
									}
									//запрещаем прерывания по захвату.
									dat_disable();
									// устанавливаем паузу.
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p;
									dat.enb_time_pause=1;
								}
								break;
							}
							case 0x03:
							{
								if(!dat.enb_time_pause) // ожидаем окончания паузы
								{
									// пауза вышла включаем все датчики
									dat.reg_rab=4;
									dat_enable();
								}
								break;
							}
							case 0x04:
							{
								if(!ftx && dat.flag==1) // сработал датчик
								{
									if(dat.dat_per==0) // сработал поле А
									{
										// передаем дачик А
										dat.count_A++;
										TXbuf[2]=0x30;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1; // передаем
										dat.reg_rab=1;		
									}
									else // сработали другие датчики
									{
										dat.count_B++;
										dat.reg_rab=3;	
									}
									dat_disable();
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p; // устанавливаем время паузы
									dat.enb_time_pause=1;
								}
								break;
							}
						}
					break;
				}
//******************************************************************************************************
//
//
//******************************************************************************************************
							case 0x55:
							{
								// обработчик подач.
									/// основной алгоритм работы мишени при подаче
									switch(dat.reg_rab)
									{
										case 0x00:
										{
											if(dat.flag==1 && ftx!=1)
											{
												if(dat.dat_per==0)
												{
													dat.count_A++;
													dat.napr_poleta=1;
													TXbuf[2]=dat.dat_per;
													TXbuf[3]=0;
													TXbuf[4]=0;
													ftx=1; // передаем номер датчика
												}
												else
												{
													dat.count_B++;
													dat.napr_poleta=0;
												}
												// запускаем таймер темпа
													dat.enb_time_temp=0;
													dat.temp_game=dat.time_t;
													dat.enb_time_temp=1;
												// запускаем таймер паузы.
												dat.enb_time_pause=0;
												dat.pause_game=dat.time_p;
												dat.enb_time_pause=1;
												dat_disable();
												dat.reg_rab=1;
											}
											break;
										}
										case 0x01:
										{
											if(!dat.enb_time_pause)
											{
												// время паузы вышло.
												if(dat.napr_poleta) dat.reg_rab=2;
												else dat.reg_rab=0;
												dat_enable();
											}
											break;
										}
										case 0x02:
										{
											if(!dat.enb_time_temp) //вышло время темпа
											{
												// переключаемся на начало.
												dat.reg_rab=0;
												break;
												
											}
											if(!ftx && dat.flag==1) // сработал датчик
											{
												if(dat.dat_per!=0)
												{
													dat.count_B++;
													TXbuf[2]=dat.dat_per;
													time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
													TXbuf[3]=(time>>8)&0xff;
													TXbuf[4]=time&0xff;
													ftx=1; // передаем номер датчика
													dat.reg_rab=3;
												}
												else
												{
													dat.count_A++;
													dat.reg_rab=1;
													dat.napr_poleta=1;
													// запускаем таймер паузы
													dat.enb_time_pause=0;
													dat.pause_game=dat.time_p;
													dat.enb_time_pause=1;
												}
												dat_disable();
												// запускаем таймер темпа
												dat.enb_time_temp=0;
												dat.temp_game=dat.time_t;
												dat.enb_time_temp=1;
											}
											break;
										}
										case 3:
										{
											if(!dat.enb_time_temp)
											{
												// время темпа вышло.
												dat.reg_rab=0;
												dat_enable();
											}
											break;
										}
									}
							break;
					}
			}
    }
    return 0;
   }