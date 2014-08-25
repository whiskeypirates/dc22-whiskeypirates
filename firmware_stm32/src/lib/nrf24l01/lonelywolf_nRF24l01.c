/*
 * lonelyWolf NRF24L01 library for STM32F1xx Series
 * stolen from https://github.com/LonelyWolf/stm32/blob/master/Nrf24l01/nRF24l01.c
 * heavily modified and cleaned up by true
 */

// nrf includes
#include <lonelywolf_nRF24L01.h>
// #include <delay.h>


// nrf addresses
uint8_t nRF24_RX_addr[_LWNRF_nRF24_RX_ADDR_WIDTH] = {'P','i','r','8','s'};
uint8_t nRF24_TX_addr[_LWNRF_nRF24_TX_ADDR_WIDTH] = {'P','i','r','8','s'};


// nrf SPI initialization with given prescaler
void nRF24_SPI_Init(uint16_t prescaler) {
	SPI_InitTypeDef spi;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_BaudRatePrescaler = prescaler;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CPHA = SPI_CPHA_1Edge;
	spi.SPI_CRCPolynomial = 7;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(_LWNRF_SPI_PORT, &spi);

	// NSS must be set to '1' due to NSS_Soft settings (otherwise it will be Multimaster mode).
	SPI_NSSInternalSoftwareConfig(_LWNRF_SPI_PORT, SPI_NSSInternalSoft_Set);
}

// GPIO and SPI initialization
void nRF24_init() {
#if _LWNRF_SPI_PORT_USE == 1
	// Set up SPI1 clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	// Set up port clocks (change as necessary)
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
#elif _LWNRF_SPI_PORT_USE == 2
	// SPI2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
#elif _LWNRF_SPI_PORT_USE == 3
	// SPI3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Disable JTAG for use PB3
#endif

	GPIO_InitTypeDef gpio;
	// Configure SPI pins
	gpio.GPIO_Speed = GPIO_Speed_40MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Pin = _LWNRF_SPI_SCK_PIN | _LWNRF_SPI_MISO_PIN | _LWNRF_SPI_MOSI_PIN;
	GPIO_Init(_LWNRF_SPI_GPIO_PORT, &gpio);

	// Configure CS pin as output with Push-Pull
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Pin = _LWNRF_SPI_CS_PIN;
	GPIO_Init(_LWNRF_SPI_CS_PORT, &gpio);

	// Configure CE pin as output with Push-Pull
	gpio.GPIO_Pin = _LWNRF_nRF24_CE_PIN;
	GPIO_Init(_LWNRF_nRF24_CE_PORT, &gpio);

	// Configure IRQ pin as input with Pull-Up
	gpio.GPIO_Pin = _LWNRF_nRF24_IRQ_PIN;
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(_LWNRF_nRF24_IRQ_PORT, &gpio);

	nRF24_SPI_Init(_LWNRF_SPI_BAUD); // Which SPI speed do we need?
	SPI_Cmd(_LWNRF_SPI_PORT, ENABLE);

	_LWNRF_CSN_H();
	_LWNRF_CE_L();
}

// Send/Receive data to nRF24L01 via SPI
// input:
//   data - byte to send
// output:
//   received byte from nRF24L01
uint8_t nRF24_ReadWrite(uint8_t data) {
	// Wait while DR register is not empty
	while (SPI_I2S_GetFlagStatus(_LWNRF_SPI_PORT, SPI_I2S_FLAG_TXE) == RESET);
	// Send byte to SPI
	SPI_I2S_SendData(_LWNRF_SPI_PORT, data);
	// Wait to receive byte
	while (SPI_I2S_GetFlagStatus(_LWNRF_SPI_PORT, SPI_I2S_FLAG_RXNE) == RESET);
	// Read byte from SPI bus
	return SPI_I2S_ReceiveData(_LWNRF_SPI_PORT);
}

// Write new value to register
// input:
//   reg - register number
//   value - new value
// output:
//   nRF24L01 status
uint8_t nRF24_RWReg(uint8_t reg, uint8_t value) {
	uint8_t status;

	_LWNRF_CSN_L();
	status = nRF24_ReadWrite(reg); // Select register
	nRF24_ReadWrite(value); // Write value to register
	_LWNRF_CSN_H();

	return status;
}

// Read nRF24L01 register
// input:
//   reg - register number
// output:
//   register value
uint8_t nRF24_ReadReg(uint8_t reg) {
	uint8_t value;

	_LWNRF_CSN_L();
	nRF24_ReadWrite(reg);
	value = nRF24_ReadWrite(0x00);
	_LWNRF_CSN_H();

	return value;
}

// Get data from nRF24L01 into buffer
// input:
//   reg - register number
//   pBuf - pointer to buffer
//   count - bytes count
// output:
//   nRF24L01 status
uint8_t nRF24_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t count) {
	uint8_t status,i;

	_LWNRF_CSN_L();
	status = nRF24_ReadWrite(reg);
	for (i = 0; i < count; i++) pBuf[i] = nRF24_ReadWrite(0);
	_LWNRF_CSN_L();

	return status;
}

// Send buffer to nRF24L01
// input:
//   reg - register number
//   pBuf - pointer to buffer
//   count - bytes count
// output:
//   nRF24L01 status
uint8_t nRF24_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t count) {
	uint8_t status,i;

	_LWNRF_CSN_L();
	status = nRF24_ReadWrite(reg);
	for (i = 0; i < count; i++) nRF24_ReadWrite(*pBuf++);
	_LWNRF_CSN_H();

	return status;
}

// Check if nRF24L01 present (send byte sequence, read it back and compare)
// return:
//   0 - looks like an nRF24L01 is online
//   1 - received sequence differs from original
uint8_t nRF24_Check(void) {
	uint8_t txbuf[5] = { 0xA9,0xA9,0xA9,0xA9,0xA9 };
	uint8_t rxbuf[5];
	uint8_t i;

	// Write fake TX address
	nRF24_WriteBuf(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_TX_ADDR, txbuf, 5);
	// Try to read TX_ADDR register
	nRF24_ReadBuf(_LWNRF_nRF24_REG_TX_ADDR, rxbuf, 5);
    // verify buffer
	for (i = 0; i < 5; i++) if (rxbuf[i] != txbuf[i]) return 1;
	// seems legit...
    return 0;
}

// Put nRF24L01 in RX mode
void nRF24_RXMode(uint8_t rx_payload_len, uint8_t channel) {
	_LWNRF_CE_L();

	// Set static RX address
	nRF24_WriteBuf(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RX_ADDR_P0,
			nRF24_RX_addr, _LWNRF_nRF24_RX_ADDR_WIDTH);
	// Enable ShockBurst for data pipe 0
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_EN_AA, 0x01);
	// Enable data pipe 0
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_EN_RXADDR, 0x01);
	// Set frequency channel X
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_CH, channel);
	// Set RX payload length (10 bytes)
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RX_PW_P0, rx_payload_len);
	// Setup: 1Mbps, 0dBm, LNA off
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_SETUP, 0x06);
	// Config: CRC on (2 bytes), Power UP, RX/TX ctl = PRX
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_CONFIG,0x0F);

	_LWNRF_CE_H();
	// Delay_us(10); // Hold CE high at least 10us
}

// Put nRF24L01 in TX mode
void nRF24_TXMode(uint8_t channel) {
	_LWNRF_CE_L();

	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_CONFIG, 0x02); // Config: Power UP
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_EN_AA, 0x01); // Enable ShockBurst for data pipe 0
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_SETUP_RETR, 0x1A); // Auto retransmit: wait 500us, 10 retries
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_CH, channel); // Set frequency channel 110 (2.510MHz)
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_SETUP, 0x06); // Setup: 1Mbps, 0dBm, LNA off
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_CONFIG, 0x0E); // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
}

// Check if data is available for reading
// return:
//   0 -> no data
//   1 -> RX_DR is set or some bytes present in FIFO
uint8_t nRF24_DataReady(void) {
    uint8_t status;

    status = nRF24_ReadReg(_LWNRF_nRF24_REG_STATUS);
    if (status & _LWNRF_nRF24_MASK_RX_DR) return 1;

    // Checking RX_DR isn't good enough, there's can be some data in FIFO
    status = nRF24_ReadReg(_LWNRF_nRF24_REG_FIFO_STATUS);

    return (status & _LWNRF_nRF24_FIFO_RX_EMPTY) ? 0 : 1;
}

uint8_t nRF24_RXPacket(uint8_t* pBuf, uint8_t rx_payload_len) {
	uint8_t status;

	// Read status register
	status = nRF24_ReadReg(_LWNRF_nRF24_REG_STATUS);

	if (status & _LWNRF_nRF24_MASK_RX_DR) {
		// pipe 0?
		if ((status & 0x0E) == 0) {
    		// read received payload from RX FIFO buffer
    		nRF24_ReadBuf(_LWNRF_nRF24_CMD_R_RX_PAYLOAD, pBuf, rx_payload_len);
    	}

		// Flush RX FIFO buffer
		nRF24_ReadWrite(_LWNRF_nRF24_CMD_FLUSH_RX);
		// Clear RX_DR, TX_DS, MAX_RT flags
		nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_STATUS, (status | 0x70));
	    //return nRF24_MASK_RX_DR;
	    return status;
    }

    // Some banana happens
	// Flush RX FIFO buffer
	nRF24_ReadWrite(_LWNRF_nRF24_CMD_FLUSH_RX);
	// Clear RX_DR, TX_DS, MAX_RT flags
	nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_STATUS, (status | 0x70));
    return status;
}

// Send data packet
// input:
//   pBuf - buffer with data to send
// return:
//   nRF24_MASK_MAX_RT - if transmit failed with maximum auto retransmit count
//   nRF24_MAX_TX_DS - if transmit succeed
//   contents of STATUS register otherwise
uint8_t nRF24_TXPacket(uint8_t * pBuf, uint8_t tx_payload_len, uint8_t channel) {
    uint8_t status;

    _LWNRF_CE_L();
    // Set static TX address
    nRF24_WriteBuf(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_TX_ADDR,
    		nRF24_TX_addr, _LWNRF_nRF24_TX_ADDR_WIDTH);
    // Set static RX address for auto ack
    nRF24_WriteBuf(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RX_ADDR_P0,
    		nRF24_RX_addr, _LWNRF_nRF24_RX_ADDR_WIDTH);
    // Enable auto acknowledgement for data pipe 0
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_EN_AA, 0x01);
    // Automatic retransmission: wait 500us, 10 retries
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_SETUP_RETR, 0x1A);
    // Set frequency channel (2400MHz + channel)
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_CH, channel);
    // Setup: 1Mbps, 0dBm, LNA on
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_RF_SETUP, 0x07);
    // Write specified buffer to FIFO
    nRF24_WriteBuf(_LWNRF_nRF24_CMD_W_TX_PAYLOAD, pBuf, tx_payload_len);
    // Config: CRC on (2 bytes), Power UP, RX/TX ctl = PTX
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_CONFIG, 0x0E);

    // CE pin high => Start transmit
    _LWNRF_CE_H();

    //Delay_us(10); // Must hold CE at least 10us
    //while(PB_IDR_bit.IDR2 != 0); // Wait for IRQ from nRF24L01
    _LWNRF_CE_L();
    // Read status register
    status = nRF24_ReadReg(_LWNRF_nRF24_REG_STATUS);
    // Clear RX_DR, TX_DS, MAX_RT flags

    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_STATUS, (status | 0x70));
    if (status & _LWNRF_nRF24_MASK_MAX_RT) {
        // Auto retransmit counter exceeds the programmed maximum limit. FIFO is not removed.
    	// Flush TX FIFO buffer
    	nRF24_RWReg(_LWNRF_nRF24_CMD_FLUSH_TX, 0xFF);
        return _LWNRF_nRF24_MASK_MAX_RT;
    }

    if (status & _LWNRF_nRF24_MASK_TX_DS) {
        // Transmit ok
    	// Flush TX FIFO buffer
    	nRF24_RWReg(_LWNRF_nRF24_CMD_FLUSH_TX,0xFF);
        return _LWNRF_nRF24_MASK_TX_DS;
    }

    // Some banana happens
    return status;
}

// Clear all IRQ flags
void nRF24_ClearIRQFlags(void) {
	uint8_t status;

    status = nRF24_ReadReg(_LWNRF_nRF24_REG_STATUS);
    // Clear RX_DR, TX_DS, MAX_RT flags
    nRF24_RWReg(_LWNRF_nRF24_CMD_WREG | _LWNRF_nRF24_REG_STATUS, (status | 0x70));
}
