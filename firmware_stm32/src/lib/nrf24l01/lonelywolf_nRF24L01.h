/*
 * lonelyWolf NRF24L01 library for STM32F1xx Series
 * stolen from https://github.com/LonelyWolf/stm32/blob/master/Nrf24l01/nRF24L01.h
 * heavily modified and cleaned up by true
 */


#ifndef __LONELYWOLF_NRF24L01_LIB
#define __LONELYWOLF_NRF24L01_LIB


// which SPI to use?
#define _LWNRF_SPI_PORT_USE 		1
#define _LWNRF_SPI_BAUD 			SPI_BaudRatePrescaler_4

// nRF24L01 CS (Chip Select) pin
#define _LWNRF_SPI_CS_PORT 			GPIOD
#define _LWNRF_SPI_CS_PIN 			GPIO_Pin_2 		// PD2

// nRF24L01 CE (Chip Enable) pin
#define _LWNRF_nRF24_CE_PORT 		GPIOC
#define _LWNRF_nRF24_CE_PIN 		GPIO_Pin_10 	// PC10

// nRF24L01 IRQ pin
#define _LWNRF_nRF24_IRQ_PORT 		GPIOC
#define _LWNRF_nRF24_IRQ_PIN 		GPIO_Pin_11 	// PC11


// include stm library
#include <stdlib.h>
#include <stdint.h>
#include <stm32l1xx.h>


// library defines
#if _LWNRF_SPI_PORT_USE == 1
	#define _LWNRF_SPI_PORT 		SPI1
	/*
	#define _LWNRF_SPI_GPIO_PORT 	GPIOA
	#define _LWNRF_SPI_SCK_PIN 		GPIO_Pin_5     // PA5
	#define _LWNRF_SPI_MISO_PIN 	GPIO_Pin_6     // PA6
	#define _LWNRF_SPI_MOSI_PIN 	GPIO_Pin_7     // PA7
	#define _LWNRF_SPI_CS_PIN 		GPIO_Pin_4     // PA4
	*/
	#define _LWNRF_SPI_GPIO_PORT 	GPIOB
	#define _LWNRF_SPI_SCK_PIN 		GPIO_Pin_3
	#define _LWNRF_SPI_MISO_PIN 	GPIO_Pin_5
	#define _LWNRF_SPI_MOSI_PIN 	GPIO_Pin_4
#elif _LWNRF_SPI_PORT_USE == 2
	#define _LWNRF_SPI_PORT 		SPI2
	#define _LWNRF_SPI_GPIO_PORT 	GPIOB
	#define _LWNRF_SPI_SCK_PIN 		GPIO_Pin_13    // PB13
	#define _LWNRF_SPI_MISO_PIN 	GPIO_Pin_14    // PB14
	#define _LWNRF_SPI_MOSI_PIN 	GPIO_Pin_15    // PB15
	#define _LWNRF_SPI_CS_PIN 		GPIO_Pin_12    // PB12
#elif _LWNRF_SPI_PORT_USE == 3
	#define _LWNRF_SPI_PORT 		SPI3
	#define _LWNRF_SPI_GPIO_PORT 	GPIOB
	#define _LWNRF_SPI_SCK_PIN 		GPIO_Pin_3     // PB3  (JTDO)
	#define _LWNRF_SPI_MISO_PIN 	GPIO_Pin_4     // PB4  (NJTRST)
	#define _LWNRF_SPI_MOSI_PIN 	GPIO_Pin_5     // PB5
	#define _LWNRF_SPI_CS_PIN 		GPIO_Pin_6     // PB6
#endif

// Chip Enable Activates RX or TX mode
#define _LWNRF_CE_L() 				GPIO_ResetBits(_LWNRF_nRF24_CE_PORT, _LWNRF_nRF24_CE_PIN)
#define _LWNRF_CE_H() 				GPIO_SetBits(_LWNRF_nRF24_CE_PORT, _LWNRF_nRF24_CE_PIN)

// SPI Chip Select
#define _LWNRF_CSN_L() 				GPIO_ResetBits(_LWNRF_SPI_CS_PORT, _LWNRF_SPI_CS_PIN)
#define _LWNRF_CSN_H() 				GPIO_SetBits(_LWNRF_SPI_CS_PORT, _LWNRF_SPI_CS_PIN)

/* nRF24L0 commands */
#define _LWNRF_nRF24_CMD_RREG             0x00  // R_REGISTER -> Read command and status registers
#define _LWNRF_nRF24_CMD_WREG             0x20  // W_REGISTER -> Write command and status registers
#define _LWNRF_nRF24_CMD_R_RX_PAYLOAD     0x61  // R_RX_PAYLOAD -> Read RX payload
#define _LWNRF_nRF24_CMD_W_TX_PAYLOAD     0xA0  // W_TX_PAYLOAD -> Write TX payload
#define _LWNRF_nRF24_CMD_FLUSH_TX         0xE1  // FLUSH_TX -> Flush TX FIFO
#define _LWNRF_nRF24_CMD_FLUSH_RX         0xE2  // FLUSH_RX -> Flush RX FIFO
#define _LWNRF_nRF24_CMD_REUSE_TX_PL      0xE3  // REUSE_TX_PL -> Reuse last transmitted payload
#define _LWNRF_nRF24_CMD_NOP              0xFF  // No operation (to read status register)

/* nRF24L0 registers */
#define _LWNRF_nRF24_REG_CONFIG           0x00  // Configuration register
#define _LWNRF_nRF24_REG_EN_AA            0x01  // Enable "Auto acknowledgment"
#define _LWNRF_nRF24_REG_EN_RXADDR        0x02  // Enable RX addresses
#define _LWNRF_nRF24_REG_SETUP_AW         0x03  // Setup of address widths
#define _LWNRF_nRF24_REG_SETUP_RETR       0x04  // Setup of automatic retranslation
#define _LWNRF_nRF24_REG_RF_CH            0x05  // RF channel
#define _LWNRF_nRF24_REG_RF_SETUP         0x06  // RF setup register
#define _LWNRF_nRF24_REG_STATUS           0x07  // Status register
#define _LWNRF_nRF24_REG_OBSERVE_TX       0x08  // Transmit observe register
#define _LWNRF_nRF24_REG_CD               0x09  // Carrier detect
#define _LWNRF_nRF24_REG_RX_ADDR_P0       0x0A  // Receive address data pipe 0
#define _LWNRF_nRF24_REG_RX_ADDR_P1       0x0B  // Receive address data pipe 1
#define _LWNRF_nRF24_REG_RX_ADDR_P2       0x0C  // Receive address data pipe 2
#define _LWNRF_nRF24_REG_RX_ADDR_P3       0x0D  // Receive address data pipe 3
#define _LWNRF_nRF24_REG_RX_ADDR_P4       0x0E  // Receive address data pipe 4
#define _LWNRF_nRF24_REG_RX_ADDR_P5       0x0F  // Receive address data pipe 5
#define _LWNRF_nRF24_REG_TX_ADDR          0x10  // Transmit address
#define _LWNRF_nRF24_REG_RX_PW_P0         0x11  // Number of bytes in RX payload id data pipe 0
#define _LWNRF_nRF24_REG_RX_PW_P1         0x12  // Number of bytes in RX payload id data pipe 1
#define _LWNRF_nRF24_REG_RX_PW_P2         0x13  // Number of bytes in RX payload id data pipe 2
#define _LWNRF_nRF24_REG_RX_PW_P3         0x14  // Number of bytes in RX payload id data pipe 3
#define _LWNRF_nRF24_REG_RX_PW_P4         0x15  // Number of bytes in RX payload id data pipe 4
#define _LWNRF_nRF24_REG_RX_PW_P5         0x16  // Number of bytes in RX payload id data pipe 5
#define _LWNRF_nRF24_REG_FIFO_STATUS      0x17  // FIFO status register
#define _LWNRF_nRF24_REG_DYNPD            0x1C  // Enable dynamic payload length
#define _LWNRF_nRF24_REG_FEATURE          0x1D  // Feature register

/* nRF24L0 bits */
#define _LWNRF_nRF24_MASK_RX_DR           0x40  // Mask interrupt caused by RX_DR
#define _LWNRF_nRF24_MASK_TX_DS           0x20  // Mask interrupt caused by TX_DS
#define _LWNRF_nRF24_MASK_MAX_RT          0x10  // Mask interrupt caused by MAX_RT
#define _LWNRF_nRF24_FIFO_RX_EMPTY        0x01  // RX FIFO empty flag
#define _LWNRF_nRF24_FIFO_RX_FULL         0x02  // RX FIFO full flag

/* Some constants */
#define _LWNRF_nRF24_RX_ADDR_WIDTH        5    // nRF24 RX address width
#define _LWNRF_nRF24_TX_ADDR_WIDTH        5    // nRF24 TX address width


/* Variables */
extern uint8_t nRF24_RX_addr[_LWNRF_nRF24_RX_ADDR_WIDTH];
extern uint8_t nRF24_TX_addr[_LWNRF_nRF24_TX_ADDR_WIDTH];


/* Function prototypes */
void nRF24_init();

uint8_t nRF24_RWReg(uint8_t reg, uint8_t value);
uint8_t nRF24_ReadReg(uint8_t reg);
uint8_t nRF24_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t count);
uint8_t nRF24_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t count);

uint8_t nRF24_Check(void);

void nRF24_RXMode(uint8_t rx_payload_len, uint8_t channel);
void nRF24_TXMode(uint8_t channel);
uint8_t nRF24_DataReady(void);

uint8_t nRF24_RXPacket(uint8_t* pBuf, uint8_t rx_payload_len);
uint8_t nRF24_TXPacket(uint8_t * pBuf, uint8_t tx_payload_len, uint8_t channel);

void nRF24_ClearIRQFlags(void);


#endif
