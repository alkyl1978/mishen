#include "stm32f0xx.h" 
#include "nrf24l01.h"
#include "nrf24l01_hw.h"
#include "nrf24l01_Wizard.h"

extern RF_TypeDef RF;
extern volatile uint8_t time_nrf;
// **************************************************************************************************
//
// начальная настройка NRF24L01
//
//***************************************************************************************************
uint8_t RF_Init()
{
	uint8_t temp;
	unsigned char string_TX[]=RFW_TX_ADDR_REG;
  unsigned char string_RX_pipe_0[]=RFW_RX_ADDR_P0_REG;
  unsigned char string_RX_pipe_1[]=RFW_RX_ADDR_P1_REG;
  unsigned char string_RX_pipe_2[]=RFW_RX_ADDR_P2_REG;
  unsigned char string_RX_pipe_3[]=RFW_RX_ADDR_P3_REG;
  unsigned char string_RX_pipe_4[]=RFW_RX_ADDR_P4_REG;
  unsigned char string_RX_pipe_5[]=RFW_RX_ADDR_P5_REG;
  time_nrf=20;
	while(time_nrf); // пауза
	RF_Send_Cmd(CONFIG_REG,RFW_CONFIG_REG);
	time_nrf=2;
	while(time_nrf); // пауза
	RF_Send_Cmd( EN_AA_REG,RFW_EN_AA_REG);
	RF_Send_Cmd( EN_RXADDR_REG,RFW_EN_RXADDR_REG);									  //EX_RXADDR		REG
	RF_Send_Cmd( SETUP_AW_REG,RFW_SETUP_AW_REG);											//SETUP_AW		REG
	RF_Send_Cmd( SETUP_RETR_REG,RFW_SETUP_RETR_REG);
	RF.CH=RFW_RF_CH_REG;
	RF_Send_Cmd( RF_CH_REG,RFW_RF_CH_REG);									         //RF Chanel		REG
	RF_Send_Cmd( RF_SETUP_REG,RFW_RF_SETUP_REG);
	RF_Send_Adrs( TX_ADDR_REG,string_TX,Max_Adress_Len); 
  RF_Send_Adrs( RX_ADDR_P0_REG,string_RX_pipe_0,Max_Adress_Len);	//RX adress for PIPE0	 
	RF_Send_Adrs( RX_ADDR_P1_REG,string_RX_pipe_1,Max_Adress_Len);  //RX adress for PIPE1	 
  RF_Send_Cmd( RX_ADDR_P2_REG,string_RX_pipe_2[0]);								//RX adress for PIPE2 (only LSB) 	  
  RF_Send_Cmd( RX_ADDR_P3_REG,string_RX_pipe_3[0]);								//RX adress for PIPE3 (only LSB)	
  RF_Send_Cmd( RX_ADDR_P4_REG,string_RX_pipe_4[0]);								//RX adress for PIPE4 (only LSB)	 
  RF_Send_Cmd( RX_ADDR_P5_REG,string_RX_pipe_5[0]);								//RX adress for PIPE5 (only LSB)      
  RF_Send_Cmd( RX_PW_P0_REG,RFW_RX_PW_P0_REG);    
  RF_Send_Cmd( RX_PW_P1_REG,RFW_RX_PW_P1_REG);    
  RF_Send_Cmd( RX_PW_P2_REG,RFW_RX_PW_P2_REG);   
  RF_Send_Cmd( RX_PW_P3_REG,RFW_RX_PW_P3_REG);   
  RF_Send_Cmd( RX_PW_P4_REG,RFW_RX_PW_P4_REG);   
  RF_Send_Cmd( RX_PW_P5_REG,RFW_RX_PW_P5_REG);
}

void RF_ReadPayload (uint8_t DataLen)
{
	uint8_t temp=0;
	while(RF.SPI_TX_ENABLE); //
	RF.nrf24l01_tx.cmd=RF_ReadPayload_CMD;
	while(DataLen!=temp)  
	{
		RF.nrf24l01_tx.buf[temp]=0xff;
		temp++;
	}
	DMA1_Channel3->CCR&=~DMA_CCR_EN;
	DMA1_Channel3->CNDTR=DataLen+1;
	DMA1_Channel2->CCR&=~DMA_CCR_EN;
	DMA1_Channel2->CNDTR=DataLen+1;
	CSN_OFF;
	DMA1_Channel2->CCR|=DMA_CCR_EN;//
	DMA1_Channel3->CCR|=DMA_CCR_EN;//
	RF.SPI_TX_ENABLE=1;
}

//***************************************************************************************************
void RF_SendPayload ( uint8_t * data, uint8_t DataLen)
{
	uint8_t temp=0;
	while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
	RF.nrf24l01_tx.cmd=RF_SendPayload_CMD;
	while(DataLen!=temp)  
	{
		RF.nrf24l01_tx.buf[temp]=*data++;
		temp++;
	}
	DMA1_Channel3->CCR&=~DMA_CCR_EN;
	DMA1_Channel3->CNDTR=DataLen+1;
	DMA1_Channel2->CCR&=~DMA_CCR_EN;
	DMA1_Channel2->CNDTR=DataLen+1;
	CSN_OFF;
	DMA1_Channel2->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
	DMA1_Channel3->CCR|=DMA_CCR_EN;//разрешаем канал ДМА
	RF.SPI_TX_ENABLE=1;
	CE_ON;
}
//*******************************************************************************************************
uint8_t RF_Read_Cmd(uint8_t adrs)
{
	while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
	RF.nrf24l01_tx.cmd=adrs;
	RF.nrf24l01_tx.buf[0]=0xff;
	Spi_send((uint32_t *)&RF.nrf24l01_tx,2);
	while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
	return RF.nrf24l01_rx[1];
}
//*******************************************************************************************************
void RF_Send_Cmd(uint8_t adrs, uint8_t cmd)
{
	while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
	RF.nrf24l01_tx.cmd=adrs|NRF_CMD_WREG;
	RF.nrf24l01_tx.buf[0]=cmd;
	Spi_send((uint32_t *)&RF.nrf24l01_tx,2);
}

//******************************************************************************************************
void RF_Send_Adrs(uint8_t adrs, uint8_t *cmd, uint8_t len)
{	
	unsigned char temp,i;
	if (cmd[0]!=0)
		{
			while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
			RF.nrf24l01_tx.cmd=adrs|NRF_CMD_WREG;
			temp=0;
			while(len!=temp)  
			{
					RF.nrf24l01_tx.buf[temp]=*cmd++;
					temp++;
			}
			Spi_send((uint32_t *)&RF.nrf24l01_tx,len+1);
		}
}
//*******************************************************************************************************
uint8_t RF_Carrier_Detect()										//returns 1 if Carrier Detected on current channel
	{return (RF_Read_Cmd( CD_REG)&0x01);}
//*******************************************************************************************************
uint8_t RF_Count_Lost_Packets()									//returns num of Lost Packatets 
	{return (((RF_Read_Cmd( OBSERV_TX_REG))&0xF0)>>4);}
//********************************************************************************************************
uint8_t RF_Count_Resend_Packets()								//returns nut of Resend Packets
	{return ((RF_Read_Cmd(OBSERV_TX_REG))&0x0F);}
//*********************************************************************************************************
uint8_t RF_IRQ_CLEAR ( unsigned char CMD)											//Clears IRQ
	{RF_Send_Cmd( STATUS_REG, CMD);} 
//*********************************************************************************************************
void RF_Flush ( unsigned char CMD)
{
	while(RF.SPI_TX_ENABLE); // ожидаем окончания приема
  RF.nrf24l01_tx.cmd=CMD;
	Spi_send((uint32_t *)&RF.nrf24l01_tx,1);
}
//*********************************************************************************************************