/* NRF24L01 Wizard Initiation Procedure v1.0 */
#include "stm32f0xx.h"
#include "nrf24l01.h"
// <<< Use Configuration Wizard in Context Menu >>>
// ================================ CONFIGURATION REGISTERS =========================================
//		<h> Basic configuration
//				<o0.0> Mode selection:  
//					<1=> Receiver
//					<0=> Transmitter
//				<o0.1> Power control
//					<1=> Ready
//					<0=> Suspend mode
//				<o0.2..3> CRC
//					<0=> CRC off
//					<1=> CRC8 on
//					<3=> CRC16 on
//				<o0.4> Maximum Retrasmit Interrupt Disable
//				<o0.5> Data Send Interrupt Disable
//				<o0.6> Data Received Interrupt Disable
//		</h>
#define RFW_CONFIG_REG					125 
// <<< end of configuration section >>> 

void Wizard_NRF24L01_Init (void)
{
	
	
}