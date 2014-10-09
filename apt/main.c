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
extern dat_def dat; // ��������� �������
int16_t crc16_modbus(unsigned char *buf,unsigned char len);
extern volatile unsigned char RXbuf[2][8]; 
extern volatile unsigned char nRXpr;               
extern volatile unsigned char fRXpr;   
extern volatile unsigned char TXbuf[8];  //���������� �����
extern volatile int16_t sec;
volatile uint8_t ftx; // ���� ��������� �������� ������
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
	 SysTick_Config(SystemCoreClock /1000);  //������� ����������
	 Uart_init();
	 dat0_init(); 
	 dat1_2_3_init();
	 dat4_5_6_7_init();
	 dat8_9_init();
	 nrf24l01_init();
	 RF_Init(); // ��������� ��������� NRF24L01
	 CE_ON;
	 time_nrf=200;
	 while(time_nrf){}; // �����
   while(1)
    {		
			// ���� ���������� ������������
			if(RF.NRF_IRQ || !(GPIOA->IDR&(1<<8)))
			{
				status=RF_Read_Cmd(STATUS_REG); // ������ ������ �������
				while(status!=0x0e)
				{
					if(status&RF_FIFO_FULL)
					{
						RF_Flush(NRF_CMD_FLUSH_RX);// ����� ����������.
						RF.NRF_RX=2; // ������������� ���� ������ �������� ������.
						status=RF_Read_Cmd(STATUS_REG); // ������ ������ �������
					}
					if(status&RF_MAX_RT_IRQ_CLEAR)
					{
						RF_Send_Cmd(STATUS_REG,RF_MAX_RT_IRQ_CLEAR);
						RF.NRF_TX=2; // ������������� ���� ������ �������� ������.
					}					
					if(status&RF_TX_DS_IRQ_CLEAR) 
					{
						RF.NRF_TX =1; // ������������� ���� ������ ��������.
						RF.NRF_RX =0;
						RF_Send_Cmd(STATUS_REG,RF_TX_DS_IRQ_CLEAR);
					}
					if(status&RF_RX_DR_IRQ_CLEAR)
					{
						dat_disable();                // ��������� �������
						RF.RX_BUSY=0;
						RF.Pid=status>>1;  // ��������� �������� ���.
						RF_ReadPayload(1); // ��������� ������
						while(!RF.RX_BUSY);  // ������� ��������� ������.
						RF.NRF_RX=1;       // ������ �������
						dat.flag=1;
						dat.dat_per=0x0a;
						RF.RX_BUSY=0;
						RF_Send_Cmd(STATUS_REG,RF_RX_DR_IRQ_CLEAR);
					}
					status=RF_Read_Cmd(STATUS_REG); // ������ ������ �������
					if(status!=0x0e)
					{
						dat_disable();                // ��������� �������
						RF.RX_BUSY=0;
						RF.Pid=status>>1;  // ��������� �������� ���.
						RF_ReadPayload(1); // ��������� ������
						while(!RF.RX_BUSY);  // ������� ��������� ������.
						RF.NRF_RX=1;       // ������ �������
						dat.flag=1;
						dat.dat_per=0x0a;
						RF.RX_BUSY=0;
						status=RF_Read_Cmd(STATUS_REG); // ������ ������ �������
					}
				}
					RF.NRF_IRQ=0;
					CE_ON; //�������� ����������������
			}
			if(DMA1_Channel4->CNDTR==0 && ftx)
			{
				DMA1_Channel4->CCR&=~DMA_CCR_EN;
				DMA1_Channel4->CNDTR=7; // ���������� ����
				// ����������� CRC
				crc=crc16_modbus((unsigned char *)&TXbuf[0],4);
				TXbuf[5]=(crc>>8)&0xff;
				TXbuf[6]=crc&0xff;
				DMA1_Channel4->CCR|=DMA_CCR_EN;//��������� ����� ���
			}
			// �������� ����� ������
			if(fRXpr)
			{
				crc=crc16_modbus((unsigned char *)&RXbuf[nRXpr][0],5);
				if(((crc>>8)&0xff)==RXbuf[nRXpr][6] && (crc&0xff)==RXbuf[nRXpr][7])
				{
					// ��������� �������� ���������
					switch (RXbuf[nRXpr][0])
					{
						case 0xCC: // ������� �������
					{
						dat.cmd=RXbuf[nRXpr][2];
						dat.time_d=RXbuf[nRXpr][3]*8; // ����� ������������
						dat.time_p=RXbuf[nRXpr][5]*16; // ����� �����
						dat.time_t=RXbuf[nRXpr][4]*8;  // ���� ����
						TXbuf[0]=dat.cmd; // ������������� ����� ����� ������
						TXbuf[1]=0x03; // ���������� ����
						//��������� ���������� �� �������.
						dat_disable();
						// ����������� � ����������� �� �������
						//***********************************************************************
						switch(dat.cmd)
						{
							//************************************************************************
							//
							//  ������ � ������� �������� ��������
							//
							//************************************************************************
							case 0xAA:
							{
								//  �������� �������� ������ �� �����
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // ��������� �������
								dat.reg_rab=0;  // ������������� ������� ����� ������.
								break;
							}
							case 0x55:
							{
								//**********************************************************************
								//
								//
								// �������������� ��������� �����.
								//
								//
								//**********************************************************************
								//  �������� �������� ������ �� �����
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // ��������� �������
								dat.reg_rab=0;  // ������������� ������� ����� ������.
								break;
							}
							case 0x99:
							{
								//*******************************************************************
								// �������������� ��������� ��� ������ � ��������� ���������
								//*******************************************************************
								//  �������� �������� ������ �� �����
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // ��������� �������
								dat.reg_rab=0;  // ������������� ������� ����� ������.
								break;
							}
							case 0x33:
							{
								//*******************************************************************
								// �������������� ��������� ��� ����� �������� � �������
								//********************************************************************
								//  �������� �������� ������ �� �����
								dat.count_A=0;
								dat.count_B=0;
								dat.count_R=0;
								dat_enable(); // ��������� �������
								dat.reg_rab=0;  // ������������� ������� ����� ������.
								break;
							}
						}
						//***********************************************************************
						break;
					}
						case 0x99: 	// ������ ���������
						{
							ftx=1; 		// �������� ��������
							break;
						}
						case 0x33: 	// ����������� ������ ������
						{
							break;
						}
					}
				}
				fRXpr=0; // ���������� ���� ������ ������ ����������
			}
/// ********************************************************************************************************
// �������� ���� ���������
//**********************************************************************************************************
			switch(dat.cmd)
			{
	//****************************************************************************************************
				//
				//
				// ���������� ������ � �������� 
//********************************************************************************************************
				case 0xAA:
				{
					switch(dat.reg_rab)
					{
						case 0:
						{
							if(dat.flag==1 && ftx!=1) // �������� ������ �����
							{
								if(dat.dat_per==0) // �������� ������� ������
								{
									if(dat.count_A==0) /// �������� ������ �
									{
										// �������� ����������� ������
										TXbuf[2]=dat.dat_per;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1;
									}
									else
									{
										if(dat.count_R!=0) // ��� ��� ���� �� ������� 
										{
											// �������� ����� �������
											TXbuf[2]=dat.dat_per;
											time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
											TXbuf[3]=(time>>8)&0xff;
											TXbuf[4]=time&0xff;
											ftx=1;
										}
									}
									dat.count_A++;
								}
								else  // ��������� ������ ������� � �������
								{
									if(dat.dat_per<0x0a) // ��������� ���� �
									{
										if(dat.count_A!=0) // ���� ������ � ������
										{
											// �������� ����� �������
											TXbuf[2]=dat.dat_per;
											time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
											TXbuf[3]=(time>>8)&0xff;
											TXbuf[4]=time&0xff;
											ftx=1;
											// ������� ������������ �����.
											dat.count_A--; 
											dat.count_R--;
										}
										if(dat.count_R>2) // ���� ��������� ������ 
										{
											dat.count_R=1; // ������� ��� ������
										}
									}
									else  // ��������� �������
									{
										if(dat.count_A!=0)
										{
											// �������� ����� �������
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
								// ������������� �����.
								dat.enb_time_pause=0;
								dat.pause_game=dat.time_p;
								dat.enb_time_pause=1;
								dat_disable(); // ��������� �������
							}
							break;
						}
						case 1: // ���������� ����.
						{
							if(!dat.enb_time_pause) // ������� ��������� �����
							{
								dat_enable(); // ��������� ��� ������� � �������
							}
							break;
						}
					}
					break;
				}
//***************************************************************************************************
// �������� ����� ����� �����
//
//
//***************************************************************************************************
				case 0x33:
				{
					switch(dat.reg_rab)
					{
						case 0:
						{
							//��������� ���� �� ����������� �������.
							if(ftx!=1 && dat.flag==1)
							{
								// �������� ���� ������
								if(dat.dat_per==0x0A) TXbuf[2]=0x21;
								else TXbuf[2]=dat.dat_per;
								TXbuf[3]=dat.dat.sec;
								TXbuf[4]=dat.dat.ms;
								ftx=1; // ������������� ���� ��������.
								dat.reg_rab=1;
								// ������������� �����.
								dat.enb_time_pause=0;
								dat.pause_game=dat.time_p;
								dat.enb_time_pause=1;
								dat_disable(); // ��������� �������
							}
							break;
						}
						case 1:
						{
							if(!dat.enb_time_pause) // ������� ��������� �����
								{
									// ����� ����� �������� ��� �������
									dat.reg_rab=0;
									// ��������� �������.
									dat_enable();
								}
							break;
						}
					}
					break;
				}
//******************************************************************************************************
//�������� �������� ������ ������ ��� 2 ������ ��������
//
//******************************************************************************************************
				case 0x99:
				{
						/// �������� �������� ������ ������ ��� 2 ������ ��������
						switch(dat.reg_rab)
						{
							case 0x00:
							{
								if(dat.flag==1) // ���� ����������� ������
								{
									if(dat.dat_per==0) // �������� ������ ������
									{
										dat.napr_poleta=1;   // ������������� ���� ����������� ������
										dat.count_A++;       // ����������� ������� ���� �
									}
									else
									{
										dat.napr_poleta=0;   //������������� ���� ������
										dat.count_B++;       // ������� ���� �
									}
									dat.reg_rab=1; // ��������� �� ������ �����	
									// ������������� �����.
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p;
									dat.enb_time_pause=1;
									// ��������� ��� ���� ��������
									dat_disable();
							  }
							  break;
							}
							case 0x01:
							{
								if(!dat.enb_time_pause) // ������� ��������� �����
								{
									// ����� ����� �������� ��� �������
									if(dat.napr_poleta) dat.reg_rab=2;
									else dat.reg_rab=0;
									// ��������� �������.
									dat_enable();
								}
								break;
							}
							case 0x02:
							{
								if(!ftx && dat.flag==1) // �������� ������
								{
									if(dat.dat_per==0) // �������� ���� �
									{
										dat.reg_rab=1;
										dat.napr_poleta=1;
										dat.count_A++;
									}
									if(dat.dat_per!=0) // ��������� ������ �������
									{
										dat.count_B++;
										// �������� ����������� ������
										TXbuf[2]=dat.dat_per;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1;
										dat.reg_rab=3;		
									}
									//��������� ���������� �� �������.
									dat_disable();
									// ������������� �����.
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p;
									dat.enb_time_pause=1;
								}
								break;
							}
							case 0x03:
							{
								if(!dat.enb_time_pause) // ������� ��������� �����
								{
									// ����� ����� �������� ��� �������
									dat.reg_rab=4;
									dat_enable();
								}
								break;
							}
							case 0x04:
							{
								if(!ftx && dat.flag==1) // �������� ������
								{
									if(dat.dat_per==0) // �������� ���� �
									{
										// �������� ����� �
										dat.count_A++;
										TXbuf[2]=0x30;
										time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
										TXbuf[3]=(time>>8)&0xff;
										TXbuf[4]=time&0xff;
										ftx=1; // ��������
										dat.reg_rab=1;		
									}
									else // ��������� ������ �������
									{
										dat.count_B++;
										dat.reg_rab=3;	
									}
									dat_disable();
									dat.enb_time_pause=0;
									dat.pause_game=dat.time_p; // ������������� ����� �����
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
								// ���������� �����.
									/// �������� �������� ������ ������ ��� ������
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
													ftx=1; // �������� ����� �������
												}
												else
												{
													dat.count_B++;
													dat.napr_poleta=0;
												}
												// ��������� ������ �����
													dat.enb_time_temp=0;
													dat.temp_game=dat.time_t;
													dat.enb_time_temp=1;
												// ��������� ������ �����.
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
												// ����� ����� �����.
												if(dat.napr_poleta) dat.reg_rab=2;
												else dat.reg_rab=0;
												dat_enable();
											}
											break;
										}
										case 0x02:
										{
											if(!dat.enb_time_temp) //����� ����� �����
											{
												// ������������� �� ������.
												dat.reg_rab=0;
												break;
												
											}
											if(!ftx && dat.flag==1) // �������� ������
											{
												if(dat.dat_per!=0)
												{
													dat.count_B++;
													TXbuf[2]=dat.dat_per;
													time=(dat.dat.sec*1000+dat.dat.ms+dat.time_p);
													TXbuf[3]=(time>>8)&0xff;
													TXbuf[4]=time&0xff;
													ftx=1; // �������� ����� �������
													dat.reg_rab=3;
												}
												else
												{
													dat.count_A++;
													dat.reg_rab=1;
													dat.napr_poleta=1;
													// ��������� ������ �����
													dat.enb_time_pause=0;
													dat.pause_game=dat.time_p;
													dat.enb_time_pause=1;
												}
												dat_disable();
												// ��������� ������ �����
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
												// ����� ����� �����.
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