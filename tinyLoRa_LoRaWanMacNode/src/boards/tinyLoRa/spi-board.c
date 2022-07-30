/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
 * 
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include "board.h"
#include "board-config.h"
#include "spi-board.h"
#include "utilities.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"


/* Having a TRANSFER_SIZE of one byte is on purpose, since the communication of multiple
 * bytes is handled in the upper layer */
#define TRANSFER_SIZE 							1

/* hardware instances of spi on RP2040 */
#define SPI_INSTANCE_1    ((spi_inst_t* const)spi1_hw)
#define SPI_INSTANCE_2    ((spi_inst_t* const)spi0_hw)

typedef struct {
	SpiId_t id;
	spi_inst_t *const spi_hw;
	uint32_t baudRate;
	spi_cpha_t clockPhase;
	spi_cpol_t clockPolarity;
	spi_order_t spiBitsOrder;
	uint8_t dataWidth;
	uint8_t masterRxData[TRANSFER_SIZE];
	uint8_t masterTxData[TRANSFER_SIZE];
} RP2040SpiHandle_t;

/**
  * Local SPI Handles
  * DO NOT CHANGE => If you need to change SPI settings, change it in board-config.h
  */

#if(RP2040_NUMBER_OF_SPI > 0)
static RP2040SpiHandle_t spiHandle0 = {
	.id = SPI_1,
	.spi_hw = SPI_INSTANCE_1
};
#endif
#if(RP2040_NUMBER_OF_SPI > 1)
static RP2040SpiHandle_t spiHandle1 = {
	.id = SPI_2,
	.spi_hw = SPI_INSTANCE_2
};
#endif


static void MapSpiIdToHandle(SpiId_t spiId, RP2040SpiHandle_t **handle);

/**
  * CAUTION:
  * The pin configuration (muxing, clock, etc.) is made with the pin_mux.* of the board.
  * You can also use the pin configuration tool of NXP.
  * This init function doesn't care about the handovered pins
  */
void SpiInit(Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso,
	PinNames sclk, PinNames nss) {

	CRITICAL_SECTION_BEGIN();

	obj->SpiId = spiId;

	RP2040SpiHandle_t *handle;
	MapSpiIdToHandle(spiId, &handle);

#if(RP2040_NUMBER_OF_SPI > 0)
	if(handle == &spiHandle0){
		handle->baudRate = RP2040_SPI1_CONFIG_BAUDRATE;
		handle->clockPhase = RP2040_SPI1_CONFIG_PHASE;
		handle->clockPolarity = RP2040_SPI1_CONFIG_POLARITY;
		handle->spiBitsOrder = RP2040_SPI1_CONFIG_DIRECTION;
		handle->dataWidth = RP2040_SPI1_CONFIG_DATAWIDTH;
	}
#endif

#if(RP2040_NUMBER_OF_SPI > 1)
	if(handle == &spiHandle1){
		handle->baudRate = RP2040_SPI2_CONFIG_BAUDRATE;
		handle->clockPhase = RP2040_SPI2_CONFIG_PHASE;
		handle->clockPolarity = RP2040_SPI2_CONFIG_POLARITY;
		handle->spiBitsOrder = RP2040_SPI2_CONFIG_DIRECTION;
		handle->dataWidth = RP2040_SPI2_CONFIG_DATAWIDTH;
	}
#endif

	/* map pin names to actual pin with the number of pins macro */
	spi_init(handle->spi_hw, handle->baudRate);
    gpio_set_function(miso - RP2040_PINS_OFFSET, GPIO_FUNC_SPI);
    gpio_set_function(mosi - RP2040_PINS_OFFSET, GPIO_FUNC_SPI);
    gpio_set_function(sclk - RP2040_PINS_OFFSET, GPIO_FUNC_SPI);
    spi_set_format(handle->spi_hw, handle->dataWidth, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	CRITICAL_SECTION_END();
}

void SpiDeInit(Spi_t *obj) {

	RP2040SpiHandle_t *handle;
	MapSpiIdToHandle(obj->SpiId, &handle);
	spi_deinit(handle->spi_hw);
}

uint16_t SpiInOut(Spi_t *obj, uint16_t outData) {

	RP2040SpiHandle_t *handle;
	MapSpiIdToHandle(obj->SpiId, &handle);

	handle->masterRxData[0] = 0x00;
	handle->masterTxData[0] = (uint8_t) (outData);

	CRITICAL_SECTION_BEGIN();

	if(spi_write_read_blocking(handle->spi_hw, handle->masterTxData, handle->masterRxData, TRANSFER_SIZE) != TRANSFER_SIZE){
		CRITICAL_SECTION_END();
		/* error */
	}

	CRITICAL_SECTION_END();

	return handle->masterRxData[0];
}

/**
  * Since the LoRaMac-node stack's SPI enum (SpiId_t) goes from SPI_1 to SPI_2,
  * but the RP2040 offers SPIs on different pin settings, a short mapping
  * is required. This method sets the pointer "handle" to the corresponding handle of the given
  * spiId defined above.
  */
static void MapSpiIdToHandle(SpiId_t spiId, RP2040SpiHandle_t **handle) {

#if(RP2040_NUMBER_OF_SPI > 0)
	if(spiHandle0.id == spiId){
		*handle = &spiHandle0;
	}
#endif
#if(RP2040_NUMBER_OF_SPI > 1)
	if(spiHandle1.id == spiId){
		*handle = &spiHandle1;
	}
#endif
}
