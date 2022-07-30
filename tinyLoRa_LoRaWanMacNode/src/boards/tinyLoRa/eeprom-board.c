/*!
 * \file      eeprom-board.c
 *
 * \brief     Target board EEPROM driver implementation
 *
 * \remark	  This board doesn't use EEPROM, but Flash instead
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include <stdbool.h>
#include "utilities.h"
#include "eeprom-board.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"

#include "RP2040-platform.h"

/************************************************
*
* CAUTION: The LoRa stack is using 16 bit addressing scheme
* while the MCU requires 32 bit. The 16 bit offset
* given to this driver are converted to a suitable
* 32 bit flash address.
*
************************************************/

static bool FlashProgrammingOnGoing = false;

/*!
 * \brief Initializes the EEPROM emulation module.
 */
void EepromMcuInit(void) {
}

/*!
 * \brief Indicates if an erasing operation is on going.
 *
 * \retval isEradingOnGoing Returns true is an erasing operation is on going.
 */
bool EepromMcuIsErasingOnGoing(void) {
  return FlashProgrammingOnGoing;
}

/*!
 * CAUTION: The address and size has to be a multiple of the page size (256 Bytes)
 */
LmnStatus_t EepromMcuWriteBuffer(uint16_t addr, uint8_t *buffer, uint16_t size) {
	  /* The MCU is working with a 32-bit address, while the stack is using a 16 bit address. We are
	   * converting the 16 bit address to 32 bit here.
     * Before data can be written into flash, the sector (4096 Bytes) should be deleted.
     * Therfore the databuffer and it's size will be aligned to blocks of 4kB. The data that's
     * already stored in the flash will be read (into ram), overwritten with the new data, the flash-sector
     * erased and then programmed freshly.
	   */
 
    uint32_t addressOffset;     /* difference between 4096 kByte alignment and given address */
    uint32_t completeSize;      /* complete buffer size (multiple of sector size 4 kB) */
    uint32_t newStartAddress;   /* new start address of buffer, aligned to 4 kB */
    uint32_t endAddress;        /* new end address, to determine how many sectors have to be erased and programmed */
    uint32_t nOfFullSectors;    /* number of secotrs that will be read, erased and programmed again */

    if(!(addr + FLASH_TARGET_OFFSET >= FLASH_ADDRESS_START && addr + FLASH_TARGET_OFFSET<= FLASH_ADDRESS_START+FLASH_NUMBER_OF_PAGES*FLASH_PAGE_SIZE)){
        /* address not within reserved memory */
        return LMN_STATUS_ERROR;
    }else{
        newStartAddress = (addr / 4096) * 4096 + FLASH_TARGET_OFFSET;
        addressOffset = addr % FLASH_SECTOR_SIZE;

        if((addressOffset + size) % FLASH_SECTOR_SIZE != 0){
            nOfFullSectors = (addressOffset + size) / FLASH_SECTOR_SIZE + 1;
        } else {
            nOfFullSectors = (addressOffset + size) / FLASH_SECTOR_SIZE;
        }

        completeSize = nOfFullSectors * FLASH_SECTOR_SIZE;
    }

    uint8_t completeBuffer[completeSize];

    FlashProgrammingOnGoing = true;

    /* store flash contents in buffer */
    if(EepromMcuReadBuffer((uint16_t)newStartAddress, completeBuffer, completeSize) != 0){
      return LMN_STATUS_ERROR;
    }

    CRITICAL_SECTION_BEGIN();
    flash_range_erase(newStartAddress, completeSize);

    /* overwrite old data with fresh user data */
    memcpy1(completeBuffer+addressOffset, buffer, size);

    flash_range_program(newStartAddress, completeBuffer, completeSize);
    CRITICAL_SECTION_END();

    FlashProgrammingOnGoing = false;

    return LMN_STATUS_OK;
}

LmnStatus_t EepromMcuReadBuffer(uint16_t addr, uint8_t *buffer, uint16_t size) {

    /* set start address of the buffer to read */
    static const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

    if(!(FLASH_TARGET_OFFSET + (uint32_t)addr >= FLASH_ADDRESS_START && FLASH_TARGET_OFFSET + (uint32_t)addr <= FLASH_ADDRESS_START+FLASH_NUMBER_OF_PAGES*FLASH_PAGE_SIZE)){
        /* address not within reserved memory */
        return LMN_STATUS_ERROR;
    }

    CRITICAL_SECTION_BEGIN();

    /* read flash and store data in buffer */
    for(int i = 0; i < size; i++){
        buffer[i] = flash_target_contents[i + addr];
    }

    CRITICAL_SECTION_END();

    return LMN_STATUS_OK;
}

void EepromMcuSetDeviceAddr(uint8_t addr) {
	for(;;){
		/* This method is not allowed on this platform. */
	}
}

LmnStatus_t EepromMcuGetDeviceAddr(void) {
  return LMN_STATUS_ERROR;
}
