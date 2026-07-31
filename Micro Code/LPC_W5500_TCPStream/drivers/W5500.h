/*  -------------------------------------------------- //
 // |                                                | //
 // |        WRITTEN BY: Mohammadreza Hosseini       | //
 // |               								 | //
 // |            E-MAIL:  mrh9977@gmail.com          | //
 // |                                                | //
 // -------------------------------------------------- */

// Version: 2.1.0

#ifndef W5500_H
#define W5500_H
#ifdef __cplusplus
extern "C" {
#endif

#include "W5500_config.h"

#include "fsl_spi.h"
#include "fsl_gpio.h"

// Common Registers
#define W5500_REG_CHIP_ID				0x0039
#define W5500_REG_MR					0x0000		// Mode Register
#define W5500_REG_PHYCFGR				0x002E		// W5500 PHY Configuration Register
#define W5500_REG_SHAR					0x0009		// Source Hardware Address Register
#define W5500_REG_SIPR					0x000F		// Source IPv4 Address Register
#define W5500_REG_GAR					0x0001		// Gateway IP Address Register
#define W5500_REG_SUBR					0x0005		// Subnet Mask Register
#define W5500_REG_RTR					0x0019		// Retry Time-value Register
// Socket Registers
#define W5500_REG_SN_MR					0x0000		// Socket n Mode Register
#define W5500_REG_SN_PORT				0x0004		// Socket n Source Port Register
#define W5500_REG_SN_DIPR				0x000C		// Socket n Destination IP Address Register
#define W5500_REG_SN_DPORT				0x0010		// Socket n Destination Port Register
#define W5500_REG_SN_RXBUF_SIZE			0x001E		// Socket n RX Buffer Size Register
#define W5500_REG_SN_TXBUF_SIZE			0x001F		// Socket n TX Buffer Size Register
#define W5500_REG_SN_CR					0x0001		// Socket n Command Register
#define W5500_REG_SN_IR					0x0002		// Socket n Interrupt Register
#define W5500_REG_SN_SR					0x0003		// Socket n Status Register
#define W5500_REG_SN_TX_FSR				0x0020		// Socket n TX Free Size Register
#define W5500_REG_SN_TX_WR	 			0x0024		// Socket n TX Write Pointer Register
#define W5500_REG_SN_RX_RSR				0x0026		// Socket n Received Size Register
#define W5500_REG_SN_RX_RD				0x0028		// Socket n RX Read Data Pointer Register

// Blocks
#define W5500_COMMON_REGS				0x00
#define W5500_SOCKET_REGS(socket)		(((socket << 2) | 0x01) & 0x1F)
#define W5500_SOCKET_TX_BUFF(socket)	(((socket << 2) | 0x02) & 0x1F)
#define W5500_SOCKET_RX_BUFF(socket)	(((socket << 2) | 0x03) & 0x1F)

// Chip ID
#define W5500_CHIP_ID					0x04

// Socket n Command Register commands
#define W5500_CR_OPEN					0x01
#define W5500_CR_LISTEN					0x02
#define W5500_CR_CLOSE					0x10
#define W5500_CR_SEND					0x20
#define W5500_CR_SEND_KEEP				0x22
#define W5500_CR_RECV					0x40

//// Interrupt Register bits
//#define W5500_IR_CON					(1U << 0)
//#define W5500_IR_DISCON					(1U << 1)
//#define W5500_IR_RECV					(1U << 2)
//#define W5500_IR_TIMEOUT				(1U << 3)
//#define W5500_IR_SEND_OK				(1U << 4)

// Status Register statuses
#define W5500_SR_CLOSED					0x00
#define W5500_SR_INIT					0x13
#define W5500_SR_LISTEN					0x14
#define W5500_SR_SYNRECV				0x16
#define W5500_SR_ESTABLISHED			0x17
#define W5500_SR_CLOSE_WAIT				0x1C

// Interrupt Register bit masks
#define W5500_IR_CON      				(1u << 0)
#define W5500_IR_DISCON   				(1u << 1)
#define W5500_IR_RECV     				(1u << 2)
#define W5500_IR_TIMEOUT  				(1u << 3)
#define W5500_IR_SENDOK   				(1u << 4)

// Typedef
typedef enum W5500_return {
	W5500_success = 0,
	W5500_fail = 1,
	W5500_invalidArgument = 2
} W5500_return_t;

typedef enum W5500_spiStatus {
	W5500_doNothing,
	W5500_writeRegs,
	W5500_readRegs,
	W5500_doingRegs
} W5500_spiStatus_t;

typedef enum W5500_status {
	W5500_tcpError,
	W5500_clientWait,
	W5500_clientConnected,
	W5500_dataReceived
} W5500_status_t;

typedef struct W5500_spiContext {
	SPI_Type *base;
	spi_master_handle_t *handle;
	uint8_t *transferBuffer;
	uint8_t srcBuff[4];
	uint8_t destBuff[4];
	uint32_t transferSize;
	volatile W5500_spiStatus_t spiStatus;
	uint32_t linkPort;
	uint32_t linkPin;
} W5500_spiContext_t;

typedef struct W5500_networkConfig {
	uint8_t gatewayIPAdd[4];
	uint8_t IPv4Add[4];
	uint8_t macAdd[6];
	uint16_t tcpPort;
} W5500_networkConfig_t;

typedef struct W5500 {
	W5500_spiContext_t con;
	W5500_networkConfig_t netConfig;
	volatile W5500_status_t status;
	volatile uint16_t txAddr;
	void (*delay_ms)(uint32_t ms);
	void (*delay_us)(uint32_t us);
} W5500_t;

typedef struct W5500_config {
    SPI_Type *spi_base;
    spi_master_handle_t *spi_handle;
    uint32_t linkPort;
	uint32_t linkPin;
    void (*delay_ms)(uint32_t);
    void (*delay_us)(uint32_t);
    uint8_t macAdd[6];
    uint8_t gatewayIPAdd[4];
    uint8_t IPv4Add[4];
    uint16_t tcpPort;
} W5500_config_t;

// Functions
W5500_return_t W5500_getDefaultConfig(W5500_config_t *config);
W5500_return_t W5500_init(W5500_t *instance, const W5500_config_t *config);
void W5500_initMinimal(W5500_t *instance);
void W5500_spiCallBack(W5500_t *instance);
bool W5500_statusReadBlocking(W5500_t *instance, uint8_t *data, uint16_t maxDataSize, uint16_t *dataSize, bool autoInit);
uint16_t W5500_dataRead(W5500_t *instance, uint8_t *data, uint16_t maxDataSize);
bool W5500_checkTXBuff(W5500_t *instance, uint16_t dataSize);
void W5500_dataWrite(W5500_t *instance, uint8_t *data, uint16_t dataSize);

#ifdef __cplusplus
}
#endif

#endif /* W5500_H */
