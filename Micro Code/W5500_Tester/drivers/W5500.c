#include "W5500.h"

// Version: 1.1

void W5500_GetDefaultConfig(W5500_t *instance, uint8_t *IPv4, uint8_t *GWIP, int32_t port, uint32_t linkPort, uint32_t linkPin) {
	if(IPv4 == NULL){
		uint8_t IP[4] = W5500_DEFAULT_IPV4_ADDRESS;
		memcpy(instance->portIP.IPv4, IP, 4);
	} else {
		memcpy(instance->portIP.IPv4, IPv4, 4);
	}
	if(GWIP == NULL){
		uint8_t GW[4] = W5500_DEFAULT_GW_ADDRESS;
		memcpy(instance->portIP.GWIP, GW, 4);
	} else {
		memcpy(instance->portIP.GWIP, GWIP, 4);
	}
	if (port < 0){
		instance->portIP.port = W5500_DEFAULT_PORT;
	} else {
		instance->portIP.port = port;
	}
	instance->con.linkPort = linkPort;
	instance->con.linkPin = linkPin;
}

bool W5500_InitFull(W5500_t *instance, SPI_Type *base, void (*delay_ms)(uint32_t ms), void (*delay_us)(uint64_t us)) {
	uint8_t macAddr[6] = {0xAA, 0xAF, 0xFA, 0xCC, 0xE3, 0x1C};
	uint8_t subr[4] = {255, 255, 255, 0};
	uint8_t port[2] = {((instance->portIP.port >> 8) & 0xFF), ((instance->portIP.port >> 0) & 0xFF)};
	uint8_t dip[4] = {192, 168, 100, 22};
	uint8_t rtr[2] = {((1000 >> 8) & 0xFF), ((1000 >> 0) & 0xFF)};
	instance->con.base = base;
	instance->delay_ms = delay_ms;
	instance->delay_us = delay_us;
	instance->con.spiStatus = do_nothing;
	instance->status = config;
	_W5500_reset(instance);
	if(instance->delay_ms != NULL)
		instance->delay_ms(300);
	if(instance->delay_us != NULL)
		instance->delay_us(300 * 1000);
	if(!_W5500_whoAmI(instance))
		return false;
	_W5500_regWrite(instance,	W5500_REG_PHYCFGR,	W5500_COMMON_REGS,		0xD8, 	 true);			// PHY set 100Mbs
	_W5500_regsWrite(instance,	W5500_REG_SHAR_S,	W5500_COMMON_REGS,		macAddr, 6,		true);	// MAC Address
	_W5500_regsWrite(instance,	W5500_REG_SUBR_S,	W5500_COMMON_REGS,		subr,	 4, 	true);	// Subnet Mask Address
	_W5500_setIPAdd(instance,	instance->portIP.IPv4,		instance->portIP.GWIP);
	_W5500_regWrite(instance,	W5500_SN_MR,		W5500_SOCKET_REGS(0),	0x01,	 true);			// Set the socket 0 mode to TCP
	_W5500_regsWrite(instance,	W5500_SN_PORT_S,	W5500_SOCKET_REGS(0),	port,	 2, 	true);	// Socket n Source Port
	_W5500_regsWrite(instance,	W5500_SN_DIPR_S,	W5500_SOCKET_REGS(0),	dip, 	 4,		true);	// Destination IP Address
	_W5500_regsWrite(instance,	W5500_SN_DPORT_S,	W5500_SOCKET_REGS(0),	port, 	 2,		true);	// Destination Port
	_W5500_regWrite(instance,	W5500_SN_RXBUF_SIZE,W5500_SOCKET_REGS(0),	0x08,	 true);			// Socket n RX Buffer Size Register
	_W5500_regWrite(instance,	W5500_SN_TXBUF_SIZE,W5500_SOCKET_REGS(0),	0x08,	 true);			// Socket n TX Buffer Size Register
	_W5500_regsWrite(instance,	W5500_RTR_S,		W5500_COMMON_REGS,		rtr, 	 2,		true);	// Retry Time-value Register

	_W5500_socketCommand(instance, 0, W5500_CR_CLOSE, true);
	_W5500_socketCommand(instance, 0, W5500_CR_OPEN, true);
	_W5500_socketCommand(instance, 0, W5500_CR_LISTEN, true);
	instance->status = clientWait;
	return true;
}

void W5500_InitMinBlocking(W5500_t *instance) {
	instance->status = config;
	_W5500_socketCommand(instance, 0, W5500_CR_CLOSE, true);
	_W5500_socketCommand(instance, 0, W5500_CR_OPEN, true);
	_W5500_socketCommand(instance, 0, W5500_CR_LISTEN, true);
	instance->status = clientWait;
}

void _W5500_regWrite(W5500_t *instance, uint16_t reg, uint8_t blockSel, uint8_t data, bool blocking) {
	spi_transfer_t xfer = {0};
	uint8_t tx[4] = {((reg >> 8) & 0xFF), ((reg >> 0) & 0xFF), 0x04 | (blockSel << 3), data};
	xfer.txData = tx;
	xfer.rxData = NULL;
	xfer.dataSize = 4;
	xfer.configFlags = kSPI_FrameAssert;
	while (instance->con.spiStatus != do_nothing);
	instance->con.spiStatus = doing_regs;
	while(SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer) != kStatus_Success);
	if(blocking) {
		while (instance->con.spiStatus != do_nothing);
	}
}

uint8_t _W5500_regRead(W5500_t *instance, uint16_t reg, uint8_t blockSel) {
	spi_transfer_t xfer = {0};
	uint8_t tx[4] = {((reg >> 8) & 0xFF), ((reg >> 0) & 0xFF), 0x00 | (blockSel << 3)};
	uint8_t rx[4];
	xfer.txData = tx;
	xfer.rxData = rx;
	xfer.dataSize = 4;
	xfer.configFlags = kSPI_FrameAssert;
	while (instance->con.spiStatus != do_nothing);
	instance->con.spiStatus = doing_regs;
	while(SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer) != kStatus_Success);
	while (instance->con.spiStatus != do_nothing);
	return rx[3];
}

void _W5500_regsWrite(W5500_t *instance, uint16_t startReg, uint8_t blockSel, uint8_t *data, size_t size, bool blocking) {
	spi_transfer_t xfer = {0};
	uint8_t tx[3] = {((startReg >> 8) & 0xFF), ((startReg >> 0) & 0xFF), 0x04 | (blockSel << 3)};
	xfer.txData = tx;
	xfer.rxData = NULL;
	xfer.dataSize = 3;
	instance->con.regsAdd = data;
	instance->con.regsSize = size;
	while (instance->con.spiStatus != do_nothing);
	instance->con.spiStatus = write_regs;
	while(SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer) != kStatus_Success);
	if(blocking) {
		while (instance->con.spiStatus != do_nothing);
	}
}

void _W5500_regsRead(W5500_t *instance, uint16_t startReg, uint8_t blockSel, uint8_t *data, size_t size, bool blocking) {
	spi_transfer_t xfer = {0};
	uint8_t tx[3] = {((startReg >> 8) & 0xFF), ((startReg >> 0) & 0xFF), 0x00 | (blockSel << 3)};
	xfer.txData = tx;
	xfer.rxData = NULL;
	xfer.dataSize = 3;
	instance->con.regsAdd = data;
	instance->con.regsSize = size;
	while (instance->con.spiStatus != do_nothing);
	instance->con.spiStatus = read_regs;
	while(SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer) != kStatus_Success);
	if(blocking) {
		while (instance->con.spiStatus != do_nothing);
	}
}

void W5500_spiCallBack(W5500_t *instance) {
	spi_transfer_t xfer = {0};
	if(instance->con.spiStatus == write_regs) {
		xfer.txData = instance->con.regsAdd;
		xfer.rxData = NULL;
		xfer.dataSize = instance->con.regsSize;
		xfer.configFlags = kSPI_FrameAssert;
		instance->con.spiStatus = doing_regs;
		SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer);
	} else if (instance->con.spiStatus == read_regs) {
		xfer.txData = NULL;
		xfer.rxData = instance->con.regsAdd;
		xfer.dataSize = instance->con.regsSize;
		xfer.configFlags = kSPI_FrameAssert;
		instance->con.spiStatus = doing_regs;
		SPI_MasterTransferNonBlocking(instance->con.base, &(instance->con.handle), &xfer);
	} else {
		instance->con.spiStatus = do_nothing;
	}
}

void _W5500_setIPAdd(W5500_t *instance, uint8_t *IPv4, uint8_t *GWIP) {
	memcpy(instance->portIP.GWIP, GWIP, 4);
	_W5500_regsWrite(instance, W5500_REG_GAR_S, W5500_COMMON_REGS, instance->portIP.GWIP, 4, true);		// GWIP Address
	memcpy(instance->portIP.IPv4, IPv4, 4);
	_W5500_regsWrite(instance, W5500_REG_SIPR_S, W5500_COMMON_REGS, instance->portIP.IPv4, 4, true);	// IPv4 Address
}

bool W5500_statusReadBlocking(W5500_t *instance, uint8_t *data, uint16_t maxDataSize, uint16_t *dataSize, bool autoInit) {
	typedef struct {
		uint8_t con:1;
		uint8_t discon:1;
		uint8_t recv:1;
		uint8_t timeout:1;
		uint8_t sendok:1;
		uint8_t reserved:3;
		uint8_t status;
	} W5500_statusRead_t;
	bool ret = false;
	W5500_statusRead_t* wsr;
	static uint8_t status[2];
	*dataSize = 0;
	wsr = (W5500_statusRead_t*)status;
	if(instance->status != tcpError) {
		_W5500_regsRead(instance, W5500_SN_IR, W5500_SOCKET_REGS(0), status, 2, true);
		_W5500_regWrite(instance, W5500_SN_IR, W5500_SOCKET_REGS(0), 0x1F, 	 true);
		if((wsr->status == W5500_SR_ESTABLISHED) && (!((GPIO_PinRead(GPIO, instance->con.linkPort, instance->con.linkPin) == 1) || (wsr->timeout)))) {
			ret = true;
			instance->status = clientConnected;
			if(wsr->recv) {
				instance->status = dataReceived;
				if((maxDataSize > 0) && (data != NULL)) {
					*dataSize = W5500_dataRead(instance, data, maxDataSize);
				}
			}
		} else if(wsr->status == W5500_SR_LISTEN) {
			instance->status = clientWait;
		} else {
			instance->status = tcpError;
		}
	}
	if((autoInit) && (instance->status == tcpError)) {
		W5500_InitMinBlocking(instance);
	}
	return ret;
}

uint16_t W5500_dataRead(W5500_t *instance, uint8_t *data, uint16_t maxDataSize) {
	uint16_t dataSize;
	uint32_t rxRDP16;
	static uint8_t regs[4];
	static uint8_t rxRDP8[2];
	_W5500_regsRead(instance, W5500_SN_RX_RSR_S, W5500_SOCKET_REGS(0), regs, 4, true);
	dataSize = (((uint16_t)regs[0]) << 8) | (((uint16_t)regs[1]) << 0);
	if(dataSize <= maxDataSize) {
		instance->status = clientConnected;
	} else {
		instance->status = dataReceived;
		dataSize = maxDataSize;
	}
	rxRDP16 = (((uint32_t)regs[2]) << 8) | (((uint32_t)regs[3]) << 0);
	if((rxRDP16 + dataSize) > 0xFFFF) {
		_W5500_regsRead(instance, rxRDP16, W5500_SOCKET_RX_BUFF(0), data, ((0xFFFF - rxRDP16) + 1), true);
		_W5500_regsRead(instance, 0, W5500_SOCKET_RX_BUFF(0), data + ((0xFFFF - rxRDP16) + 1), dataSize - ((0xFFFF - rxRDP16) + 1), true);
	} else {
		_W5500_regsRead(instance, rxRDP16, W5500_SOCKET_RX_BUFF(0), data, dataSize, true);
	}
	rxRDP16 += dataSize;
	rxRDP8[0] = (rxRDP16 >> 8) & 0xFF;
	rxRDP8[1] = (rxRDP16 >> 0) & 0xFF;
	_W5500_regsWrite(instance, W5500_SN_RX_RD_S, W5500_SOCKET_REGS(0), rxRDP8, 2, true);
	_W5500_socketCommand(instance, 0, W5500_CR_RECV, true);
	return dataSize;
}

void W5500_dataWrite(W5500_t *instance, uint8_t *data, uint16_t dataSize) {
	uint32_t txWDP16;
	static uint8_t txWDP8[2];
	_W5500_regsRead(instance, W5500_SN_TX_WR_S, W5500_SOCKET_REGS(0), txWDP8, 2, true);
	txWDP16 = (((uint32_t)txWDP8[0]) << 8) | (((uint32_t)txWDP8[1]) << 0);
	if((txWDP16 + dataSize) > 0xFFFF) {
		_W5500_regsWrite(instance, txWDP16, W5500_SOCKET_TX_BUFF(0), data, ((0xFFFF - txWDP16) + 1), true);
		_W5500_regsWrite(instance, 0, W5500_SOCKET_TX_BUFF(0), data + ((0xFFFF - txWDP16) + 1), dataSize - ((0xFFFF - txWDP16) + 1), true);
	} else {
		_W5500_regsWrite(instance, txWDP16, W5500_SOCKET_TX_BUFF(0), data, dataSize, true);
	}
	txWDP16 += dataSize;
	txWDP8[0] = (txWDP16 >> 8) & 0xFF;
	txWDP8[1] = (txWDP16 >> 0) & 0xFF;
	_W5500_regsWrite(instance, W5500_SN_TX_WR_S, W5500_SOCKET_REGS(0), txWDP8, 2, true);
	_W5500_socketCommand(instance, 0, W5500_CR_SEND, true);
}
