/*!
 * \file      RP2040-platform.h
 *
 * \brief     General configuration for tinyLoRa using the RP2040-Chip
 *
 * \copyright SPDX-License-Identifier: BSD-3-Clause
 *
 * \author    Julian Staffelbach (HSLU)
 */


/* FLASH configuration */
/************************************************ FLASH CNFIGURATION
*
* Total Flash Size: 16 MByte (only 2 MByte usable for now) (256 kByte available for customer)
* Block Size: 65'536 Bytes
* Sector Size: 4096 Bytes (minimum erease size)
* Page Size: 256 Bytes (minimum program size)
*
************************************************/

/*
 * starting at 2048 kByte - 256 kByte = 1792 kByte, must align to 4048 bytes -> offset is 1.75 MByte 
 * 256 * 4 = 1024 (align to 256 Bytes)
 * 14 * 1024 * 1024 = 14 MByte
 */

#define FLASH_TARGET_OFFSET       (2 * 1024 * 1024 - 16 * 4096)   /* 16 blocks for customer, flash is addressd with uint16_t, 65'526 / 4096 = 16 */
#define FLASH_ADDRESS_START       FLASH_TARGET_OFFSET
#define FLASH_NUMBER_OF_PAGES     (( 16 * 4096 ) / FLASH_PAGE_SIZE )

#define PL_CONFIG_USE_LED1        (1)

#define PL_CONFIG_USE_TESTPIN3    (0)