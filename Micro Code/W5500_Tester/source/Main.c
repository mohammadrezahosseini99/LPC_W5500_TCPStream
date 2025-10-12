/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_spi.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "W5500.h"

#include <cr_section_macros.h>
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER          SPI6
#define EXAMPLE_SPI_MASTER_IRQ      FLEXCOMM6_IRQn
#define EXAMPLE_SPI_MASTER_CLK_SRC  kCLOCK_Flexcomm6
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFlexCommClkFreq(6)
#define EXAMPLE_SPI_SSEL            1
#define EXAMPLE_SPI_SPOL            kSPI_SpolActiveAllLow

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t srcBuff[BUFFER_SIZE];
static uint8_t destBuff[BUFFER_SIZE];

volatile uint64_t g_systickCounter = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
#define MAX_REC		200

W5500_status_t status;
uint8_t recData[MAX_REC];
uint16_t recSize;
spi_master_handle_t spi_handle;

__DATA(SRAM2) W5500_t myW5500;

static void w5500_spi_Callback(SPI_Type *base, spi_master_handle_t *handle,
		status_t status, void *userData) {
	W5500_spiCallBack(&myW5500);
}

void SysTick_DelayTicks(uint64_t n) {
	g_systickCounter = n;
	while (g_systickCounter > 0);
}

void SysTick_Handler(void) {
	if (g_systickCounter != 0U)
		g_systickCounter--;
}

int main(void)
{
    spi_master_config_t userConfig = {0};
    uint32_t srcFreq               = 0;
    uint32_t i                     = 0;
    uint32_t err                   = 0;
    spi_transfer_t xfer            = {0};

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI3 */
    CLOCK_AttachClk(kSYS_PLL_to_FLEXCOMM6);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC5_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    SysTick_Config(SystemCoreClock / 1000U);
	SysTick_DelayTicks(500);

    SPI_MasterGetDefaultConfig(&userConfig);
    srcFreq            = EXAMPLE_SPI_MASTER_CLK_FREQ;
    userConfig.sselNum = (spi_ssel_t)EXAMPLE_SPI_SSEL;
    userConfig.sselPol = (spi_spol_t)EXAMPLE_SPI_SPOL;
    userConfig.baudRate_Bps = 20000000U;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &userConfig, srcFreq);
    SPI_MasterTransferCreateHandle(EXAMPLE_SPI_MASTER, &spi_handle, w5500_spi_Callback, NULL);

    W5500_GetDefaultConfig(&myW5500, NULL, NULL, NULL, -1, BOARD_INITPINS_ETH_LINK_PORT, BOARD_INITPINS_ETH_LINK_PIN);
    while(!W5500_InitFull(&myW5500, EXAMPLE_SPI_MASTER, &spi_handle, SysTick_DelayTicks, NULL));

    SysTick_DelayTicks(100);
    uint16_t cnt = 0, a;
    uint8_t hi[9] = {'H', 'i', ' ', 'B', 'r', 'o', ' ', 33, '\n'}; // "Salam.\n";
    uint8_t recDataBack[MAX_REC + 10]= {'R', 'e', 'c', 'e', 'i', 'v', 'e', 'd', ':', ' '};

    while (1)
    {
    	SysTick_DelayTicks(20);
    	W5500_statusReadBlocking(&myW5500, recDataBack + 10, MAX_REC, &recSize, true);
    	status = myW5500.status;
    	if(recSize) {
    		a++;
    		W5500_dataWrite(&myW5500, recDataBack, recSize + 10);
    		cnt = 0;
    	}
//    	if(myW5500.status == tcpError){
//    		W5500_InitMinBlocking(&myW5500);
//    		status = myW5500.status;
//    	}
    	if(myW5500.status == clientConnected) {
			cnt++;
			if(cnt >= 50){
				cnt = 0;
				hi[7]++;
				if(hi[7] == 127)
					hi[7] = 33;
				W5500_dataWrite(&myW5500, hi, 9);
			}
    	}
    	else {
    		cnt = 0;
    	}
    }
}
