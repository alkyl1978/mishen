#ifndef NRF_H
#define NRF_H

#define Max_Adress_Len 5
#define Max_Dala_Len 32			//Bytes in Payload

//****************************************************************************************************
 /*
 * NRF2401 Commands
 */
#define NRF_CMD_RREG		0x00
#define NRF_CMD_WREG		0x20
#define RF_SendPayload_CMD    		0xa0
#define RF_ReadPayload_CMD    		0x61
#define NRF_CMD_NOP	      0xff
#define NRF_CMD_FLUSH_TX	0xe1
#define NRF_CMD_FLUSH_RX	0xe2
#define NRF_CMD_WACKPL		0xa8
#


#define RF_RX_DR_IRQ_CLEAR			0x40
#define RF_TX_DS_IRQ_CLEAR			0x20
#define RF_MAX_RT_IRQ_CLEAR			0x10
#define RF_FIFO_FULL			      0x01
//****************************************************************************************************

//****************************************************************************************************

#define CSN_ON GPIOA->BSRR|=GPIO_BSRR_BS_12
#define CSN_OFF  GPIOA->BSRR|=GPIO_BSRR_BR_12
#define CE_ON GPIOA->BSRR|=GPIO_BSRR_BS_11
#define CE_OFF  GPIOA->BSRR|=GPIO_BSRR_BR_11



// NRF24l01 registers define
#define CONFIG_REG			  0x00
#define EN_AA_REG			    0x01
#define EN_RXADDR_REG		  0x02
#define SETUP_AW_REG		  0x03
#define	SETUP_RETR_REG		0x04
#define RF_CH_REG			    0x05
#define RF_SETUP_REG		  0x06
#define STATUS_REG			  0x07
#define OBSERV_TX_REG		  0x08
#define CD_REG				    0x09
#define RX_ADDR_P0_REG		0x0A
#define RX_ADDR_P1_REG		0x0B
#define RX_ADDR_P2_REG		0x0C
#define RX_ADDR_P3_REG		0x0D
#define RX_ADDR_P4_REG		0x0E
#define RX_ADDR_P5_REG		0x0F
#define TX_ADDR_REG			0x10
#define RX_PW_P0_REG		0x11
#define RX_PW_P1_REG		0x12
#define RX_PW_P2_REG		0x13
#define RX_PW_P3_REG		0x14
#define RX_PW_P4_REG		0x15
#define RX_PW_P5_REG		0x16
#define FIFO_STATUS_REG		0x17


void nrf24l01_init(void);
uint8_t Spi1_send (uint8_t data);

void RF_SendPayload ( uint8_t * data, uint8_t DataLen);
uint8_t RF_Read_Cmd(uint8_t adrs);
void RF_Send_Cmd(uint8_t adrs, uint8_t cmd);
void RF_Send_Adrs(uint8_t adrs, uint8_t *cmd, uint8_t len);
uint8_t RF_Init();
uint8_t RF_Carrier_Detect();
uint8_t RF_Count_Lost_Packets();
uint8_t RF_Count_Resend_Packets();
uint8_t RF_IRQ_CLEAR ( uint8_t CMD);
void RF_Flush ( unsigned char CMD);
void RF_ReadPayload (uint8_t DataLen);

typedef struct
{
	volatile uint8_t cmd;
	volatile uint8_t buf[32];
} RF_bufdef;

typedef struct
{
 volatile uint8_t flag;
 volatile uint8_t CH;
 volatile uint8_t ERR;           // количество пакетов
 volatile uint8_t TX_RX;               // режим работы приемопередатчика
 volatile uint8_t SPI_TX_ENABLE;       // готовы данные для передачи
 volatile uint8_t RX_PRIN;             // данные приняты
 volatile uint8_t RX_BUSY;             // идет прием данных
 volatile uint8_t SPI_TX_COUNT;        // количество данных для передачи
 volatile uint8_t NRF_RX;              // данные приняты.
 volatile uint8_t NRF_TX;              // данные переданы.
 volatile uint8_t NRF_IRQ;             // флаг прерывания от приемопередатчика
 volatile uint8_t Pid;                 // принятый пид
 RF_bufdef nrf24l01_tx;                // буфер передачи по SPI
 volatile uint8_t nrf24l01_rx[32];     // буфер приема по SPI	
 volatile uint8_t Enab_raket;          // ракетка подключена
} RF_TypeDef;


#endif