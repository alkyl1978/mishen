#ifndef __NRF24L01REGS_H_
#define __NRF24L01REGS_H_

/*
 * Setting this to 1 enables compilation of meta information
 * for the registers. Meta information contains readable register
 * names which could be used for debugging or displaying.
 * <br/><br/>
 * NOTE: Use "-DNRF_REG_DEF_META" compiler switch from makefile
 * is the preferred way to set this flag.
 */
// #define NRF_REG_DEF_META		1

/**
 * Maximum number of bytes needed to store register information (largest
 * block of data to be read from nNRF is the address register with 5 bytes).
 */
#define NRF_MAX_REG_BUF			5

/*
 * NRF2041 Registers
 */

#define CONFIG_REG			0x00
#define EN_AA_REG			0x01
#define EN_RXADDR_REG		        0x02
#define SETUP_AW_REG    		0x03
#define SETUP_RETR_REG   		0x04
#define RF_CH_REG			0x05
#define RF_SETUP_REG    		0x06
#define STATUS_REG			0x07
#define OBSERVE_TX_REG		0x08
#define CD_REG			0x09
#define RX_ADDR_P0_REG		0x0A
#define RX_ADDR_P1_REG		0x0B
#define RX_ADDR_P2_REG		0x0C
#define RX_ADDR_P3_REG		0x0D
#define RX_ADDR_P4_REG		0x0E
#define RX_ADDR_P5_REG		0x1F
#define TX_ADDR_REG	        0x10
#define RX_PW_P0_REG		0x11
#define RX_PW_P1_REG		0x12
#define RX_PW_P2_REG		0x13
#define RX_PW_P3_REG		0x14
#define RX_PW_P4_REG		0x15
#define RX_PW_P5_REG		0x16
#define FIFO_STATUS_REG		0x17
// N/A				0x18
// N/A				0x19
// N/A				0x1A
// N/A				0x1B
#define DYNPD_REG		0x1C
#define FEATURE_REG		0x1D

/*
 * NRF2401 Register Fields
 */

// CONFIG
#define NRF_REGF_PRIM_RX			0
#define NRF_REGF_PWR_UP				1
#define NRF_REGF_CRCO				2
#define NRF_REGF_EN_CRC				3
#define NRF_REGF_MASK_MAX_RT		        4
#define NRF_REGF_MASK_TX_DS			5
#define NRF_REGF_MASK_RX_DR			6

// EN_AA
#define NRF_REGF_ENAA_P0			0
#define NRF_REGF_ENAA_P1			1
#define NRF_REGF_ENAA_P2			2
#define NRF_REGF_ENAA_P3			3
#define NRF_REGF_ENAA_P4			4
#define NRF_REGF_ENAA_P5			5

// EN_RXADDR
#define NRF_REGF_ERX_P0				0
#define NRF_REGF_ERX_P1				1
#define NRF_REGF_ERX_P2				2
#define NRF_REGF_ERX_P3				3
#define NRF_REGF_ERX_P4				4
#define NRF_REGF_ERX_P5				5

// SETUP_AW
#define NRF_REGF_AW				0

// SETUP_RETR
#define NRF_REGF_ARC				0
#define NRF_REGF_ARD				1

// RF_CH
#define NRF_REGF_RF_CH				0

// RF_SETUP
#define NRF_REGF_LNA_HCURR			0
#define NRF_REGF_RF_PWR				1
#define NRF_REGF_RF_DR				2
#define NRF_REGF_PLL_LOCK			3

// STATUS
#define NRF_REGF_TX_FULL			0
#define NRF_REGF_RX_P_NO			1
#define NRF_REGF_MAX_RT				2
#define NRF_REGF_TX_DS				3
#define NRF_REGF_RX_DR				4

// OBSERVE_TX
#define NRF_REGF_ARC_CNT			0
#define NRF_REGF_PLOS_CNT			1

// CD
#define NRF_REGF_CD				0

// ADDR
#define NRF_REGF_ADDR_A				0
#define NRF_REGF_ADDR_B				1
#define NRF_REGF_ADDR_C				2
#define NRF_REGF_ADDR_D				3
#define NRF_REGF_ADDR_E				4

// RX_PW
#define NRF_REGF_PW				0

// FIFO_STATUS
#define NRF_REGF_FIFO_RX_EMPTY		0
#define NRF_REGF_FIFO_RX_FULL		1
#define NRF_REGF_FIFO_TX_EMPTY		4
#define NRF_REGF_FIFO_TX_FULL		5
#define NRF_REGF_FIFO_TX_REUSE		6

// DYNPD
#define NRF_REGF_DPL_P0			0
#define NRF_REGF_DPL_P1			1
#define NRF_REGF_DPL_P2			2
#define NRF_REGF_DPL_P3			3
#define NRF_REGF_DPL_P4			4
#define NRF_REGF_DPL_P5			5

// FEATURE
#define NRF_REGF_EN_DYN_ACK		0
#define NRF_REGF_EN_ACK_PAY		1
#define NRF_REGF_EN_DPL			2

#endif
