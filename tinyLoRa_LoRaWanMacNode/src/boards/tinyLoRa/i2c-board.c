/*!
 * \file      i2c-board.c
 *
 * \brief     Target board I2C driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include "utilities.h"
#include "board-config.h"
#include "i2c-board.h"
#include "sx-i2c.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* hardware instances of i2c on RP2040 */
#define I2C_INSTANCE_1		(&i2c0_inst)
#define I2C_INSTANCE_2		(&i2c1_inst)

typedef struct {
	I2cId_t id;
	i2c_inst_t* i2c_hw;
	uint32_t baudrate;
	uint8_t i2cInternalAddrSize;
} RP2040I2cHandle_t;

/**
  * Local I2C Handles
  * DO NOT CHANGE => If you need to change I2C settings, change it in board-config.h
  */
#if(RP2040_NUMBER_OF_I2C > 0)
static RP2040I2cHandle_t i2cHandle0 = {
	.id = I2C_1,
	.i2c_hw = I2C_INSTANCE_1
};
#endif
#if(RP2040_NUMBER_OF_I2C > 1)
static RP2040I2cHandle_t i2cHandle1 = { 
	.id = I2C_2,
	.i2c_hw = I2C_INSTANCE_2
};
#endif

static void MapI2cIdToHandle(I2cId_t i2cId, RP2040I2cHandle_t **handle);

void I2cMcuInit(I2c_t *obj, I2cId_t i2cId, PinNames scl, PinNames sda) {
	
	CRITICAL_SECTION_BEGIN();

	obj->I2cId = i2cId;
	RP2040I2cHandle_t *handle;
	MapI2cIdToHandle(i2cId, &handle);

	#if(RP2040_NUMBER_OF_I2C > 0)
		if(handle == &i2cHandle0){
			handle->baudrate = RP2040_I2C1_BAUDRATE;
		}
	#endif

	#if(RP2040_NUMBER_OF_I2C > 1)
		if(handle == &i2cHandle1){
			handle->baudrate = RP2040_I2C2_BAUDRATE;
		}
	#endif

	i2c_init(handle->i2c_hw, handle->baudrate);
	gpio_set_function(scl - RP2040_PINS_OFFSET, GPIO_FUNC_I2C);
	gpio_set_function(sda - RP2040_PINS_OFFSET, GPIO_FUNC_I2C);
	gpio_pull_up(scl - RP2040_PINS_OFFSET);
	gpio_pull_up(sda - RP2040_PINS_OFFSET);

	CRITICAL_SECTION_END();
}

void I2cMcuFormat(I2c_t *obj, I2cMode mode, I2cDutyCycle dutyCycle,
bool I2cAckEnable, I2cAckAddrMode AckAddrMode, uint32_t I2cFrequency) {
	/*
	 * This function doesn't do anything!
	 * All of this is done in I2cMcuInit()
	 */
}

void I2cMcuResetBus(I2c_t *obj){	
}

void I2cMcuDeInit(I2c_t *obj) {

	RP2040I2cHandle_t *handle;
	MapI2cIdToHandle(obj->I2cId, &handle);

	i2c_deinit(handle->i2c_hw);
}

void I2cSetAddrSize(I2c_t *obj, I2cAddrSize addrSize) {

	RP2040I2cHandle_t *handle;
	MapI2cIdToHandle(obj->I2cId, & handle);

	handle->i2cInternalAddrSize = addrSize;
}

LmnStatus_t I2cMcuWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size ) {
	
	RP2040I2cHandle_t *handle;
	MapI2cIdToHandle(obj->I2cId, &handle);

	i2c_write_blocking(handle->i2c_hw, deviceAddr, buffer, size, false);
	return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size ) {
	
	RP2040I2cHandle_t *handle;
	MapI2cIdToHandle(obj->I2cId, &handle);

	i2c_read_blocking(handle->i2c_hw, deviceAddr, buffer, size, false);
	return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuWaitStandbyState(I2c_t *obj, uint8_t deviceAddr) {
	/* Function not implemented -> Always returns successfully */
	return LMN_STATUS_OK;
}

/*!
 * Since the LoRaMac-node stack's I2C enum (I2cId_t) goes from I2C_1 to I2C_2,
 * but the RP2040 offers several I2Cs on different pin settings,a short mapping is 
 * required. This method set the pointer "handle" to the corresponding handle 
 * of the given i2cId.
 */
static void MapI2cIdToHandle(I2cId_t i2cId, RP2040I2cHandle_t **handle) {

#if(RP2040_NUMBER_OF_I2C > 0)
	if(i2cHandle0.id == i2cId){
		*handle = &i2cHandle0;
		return;
	}
#endif
#if(RP2040_NUMBER_OF_I2C > 1)
	if(i2cHandle1.id == i2cId){
		*handle = &i2cHandle1;
		return;
	}
#endif
	*handle = NULL;

}
