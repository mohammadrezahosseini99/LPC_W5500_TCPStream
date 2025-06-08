/*  -------------------------------------------------- //
 // |                                                | //
 // |        WRITTEN BY: Mohammadreza Hosseini       | //
 // |               								 | //
 // |            E-MAIL:  mrh9977@gmail.com          | //
 // |                                                | //
 // -------------------------------------------------- */

// Version: 1.1

#ifndef __W5500_H
#define __W5500_H
#ifdef __cplusplus
extern "C" {
#endif

#include "fsl_spi.h"
#include "fsl_gpio.h"
#include "W5500_config.h"

// Registers
// Common
#define W5500_REG_CHIP_ID			0x0039
#define W5500_REG_MR				0x0000		// Mode Register
#define W5500_REG_PHYCFGR			0x002E		// W5500 PHY Configuration Register
#define W5500_REG_SHAR_S			0x0009		// Source Hardware Address Register
#define W5500_REG_SIPR_S			0x000F		// Source IPv4 Address Register
#define W5500_REG_GAR_S				0x0001		// Gateway IP Address Register
#define W5500_REG_SUBR_S			0x0005		// Subnet Mask Register
#define W5500_RTR_S					0x0019		// Retry Time-value Register
// Socket
#define W5500_SN_MR					0x0000		// Socket n Mode Register
#define W5500_SN_PORT_S				0x0004		// Socket n Source Port Register
#define W5500_SN_DIPR_S				0x000C		// Socket n Destination IP Address Register
#define W5500_SN_DPORT_S			0x0010		// Socket n Destination Port Register
#define W5500_SN_RXBUF_SIZE			0x001E		// Socket n RX Buffer Size Register
#define W5500_SN_TXBUF_SIZE			0x001F		// Socket n TX Buffer Size Register
#define W5500_SN_CR					0x0001		// Socket n Command Register
#define W5500_SN_IR					0x0002		// Socket n Interrupt Register
#define W5500_SN_SR					0x0003		// Socket n Status Register
#define W5500_SN_TX_FSR_S			0x0020		// Socket n TX Free Size Register
#define W5500_SN_TX_WR_S 			0x0024		// Socket n TX Write Pointer Register
#define W5500_SN_RX_RSR_S			0x0026		// Socket n Received Size Register
#define W5500_SN_RX_RD_S			0x0028		// Socket n RX Read Data Pointer Register

// Blocks
#define W5500_COMMON_REGS			0x00
#define W5500_SOCKET_REGS(x)		(((x << 2) | 0x01) & 0x1F)
#define W5500_SOCKET_TX_BUFF(x)		(((x << 2) | 0x02) & 0x1F)
#define W5500_SOCKET_RX_BUFF(x)		(((x << 2) | 0x03) & 0x1F)

// Chip ID
#define W5500_CHIP_ID				0x04

// Socket n Command Register commands
#define W5500_CR_OPEN				0x01
#define W5500_CR_LISTEN				0x02
#define W5500_CR_CLOSE				0x10
#define W5500_CR_SEND				0x20
#define W5500_CR_SEND_KEEP			0x22
#define W5500_CR_RECV				0x40

//// Interrupt Register bits
//#define W5500_IR_CON				(1U << 0)
//#define W5500_IR_DISCON				(1U << 1)
//#define W5500_IR_RECV				(1U << 2)
//#define W5500_IR_TIMEOUT			(1U << 3)
//#define W5500_IR_SEND_OK			(1U << 4)

// Status Register statuses
#define W5500_SR_CLOSED				0x00
#define W5500_SR_INIT				0x13
#define W5500_SR_LISTEN				0x14
#define W5500_SR_ESTABLISHED		0x17
#define W5500_SR_CLOSE_WAIT			0x1C


// Typedef
typedef enum {
	do_nothing,
	write_regs,
	read_regs,
	doing_regs
} W5500_spiStatus_t;

typedef enum {
	config,
	tcpError,
	clientWait,
	clientConnected,
	dataReceived
} W5500_status_t;

typedef struct {
	SPI_Type *base;
	spi_master_handle_t *handle;
	uint8_t *regsAdd;
	size_t regsSize;
	volatile W5500_spiStatus_t spiStatus;
	uint32_t linkPort;
	uint32_t linkPin;
} W5500_con_t;

typedef struct {
	uint8_t GWIP[4];
	uint8_t IPv4[4];
	uint16_t port;
} W5500_portIP_t;

typedef struct {
	W5500_con_t con;
	W5500_portIP_t portIP;
	volatile W5500_status_t status;
	void (*delay_ms)(uint64_t ms);
	void (*delay_us)(uint64_t us);
} W5500_t;

// Functions
void W5500_GetDefaultConfig(W5500_t *instance, uint8_t *IPv4, uint8_t *GWIP, int32_t port, uint32_t linkPort, uint32_t linkPin);
bool W5500_InitFull(W5500_t *instance, SPI_Type *base, spi_master_handle_t *handle, void (*delay_ms)(uint64_t ms), void (*delay_us)(uint64_t us));
void W5500_InitMinBlocking(W5500_t *instance);
void W5500_spiCallBack(W5500_t *instance);
bool W5500_statusReadBlocking(W5500_t *instance, uint8_t *data, uint16_t maxDataSize, uint16_t *dataSize, bool autoInit);
uint16_t W5500_dataRead(W5500_t *instance, uint8_t *data, uint16_t maxDataSize);
void W5500_dataWrite(W5500_t *instance, uint8_t *data, uint16_t dataSize);

void _W5500_regWrite(W5500_t *instance, uint16_t reg, uint8_t blockSel, uint8_t data, bool blocking);
uint8_t _W5500_regRead(W5500_t *instance, uint16_t reg, uint8_t blockSel);
void _W5500_regsWrite(W5500_t *instance, uint16_t startReg, uint8_t blockSel, uint8_t *data, size_t size, bool blocking);
void _W5500_regsRead(W5500_t *instance, uint16_t startReg, uint8_t blockSel, uint8_t *data, size_t size, bool blocking);
void _W5500_setIPAdd(W5500_t *instance, uint8_t *IPv4, uint8_t *GWIP);

static inline void _W5500_reset(W5500_t *instance) {
	_W5500_regWrite(instance, W5500_REG_MR, W5500_COMMON_REGS, 0x80, true);
}

static inline bool _W5500_whoAmI(W5500_t *instance) {
	return (_W5500_regRead(instance, W5500_REG_CHIP_ID, W5500_COMMON_REGS) == W5500_CHIP_ID);
}

static inline void _W5500_socketCommand(W5500_t *instance, uint8_t socketNum, uint8_t command, bool blocking) {
	_W5500_regWrite(instance, W5500_SN_CR, W5500_SOCKET_REGS(socketNum), command, blocking);
}

#ifdef __cplusplus
}
#endif

#endif /* __W5500_H */
