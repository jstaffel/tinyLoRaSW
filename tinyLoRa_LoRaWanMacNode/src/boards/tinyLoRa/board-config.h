/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define BOARD_CONFIG_HAS_GNSS             (0)
  /*!< if board has a GNSS (GPS) or not */

#define BOARD_CONFIG_HAS_SECURE_ELEMENT   (0)
  /*!< if board has a secure element or not */

#define BOARD_CONFIG_ENTER_LOW_POWER      (0)
  /*!< if we enter low power mode */

/**
 * General definitions
 */
#define BOARD_UART_BAUDRATE						115200

/**
 * UART definitions
 */
#define RP2040_NUMBER_OF_USARTS                    0

#if(RP2040_NUMBER_OF_USARTS > 0)
#define RP2040_USART1_BAUDRATE                     BOARD_UART_BAUDRATE
#define RP2040_USART1_DATA_BITS					   8
#define RP2040_USART1_STOP_BITS					   1
#define RP2040_USART1_PARITY					   UART_PARITY_NONE
#define RP2040_USART1_IRQn                         UART0_IRQ
#define RP2040_USART1_IRQ_RX_ENABLE                1
#define RP2040_USART1_IRQ_TX_ENABLE                0
#endif
#if(RP2040_NUMBER_OF_USARTS > 1)
#define RP2040_USART2_BAUDRATE					   BOARD_UART_BAUDRATE
#define RP2040_USART2_DATA_BITS					   8
#define RP2040_USART2_STOP_BITS					   1
#define RP2040_USART2_PARITY					   UART_PARITY_NONE
#define RP2040_USART2_IRQn						   UART1_IRQ
#define RP2040_USART2_IRQ_RX_ENABLE                1
#define RP2040_USART2_IRQ_TX_ENABLE                0
#endif

/**
 * SPI definitions
 */
#define RP2040_NUMBER_OF_SPI                       1

#if(RP2040_NUMBER_OF_SPI > 0)
  /* LoRa Transceiver is on FlexComm 8 */
  #define RP2040_SPI1_TYPE                           SPI_INSTANCE_1 /* LoRa Transceiver */
  #if 0
  // not needed on tinyLoRa
  #define RP2040_SPI1_CLK_FRQ                        CLOCK_GetFlexCommClkFreq(8U)
  #endif
  #define RP2040_SPI1_CONFIG_POLARITY                SPI_CPOL_0
  #define RP2040_SPI1_CONFIG_PHASE                   SPI_CPHA_0
  #define RP2040_SPI1_CONFIG_DIRECTION               SPI_MSB_FIRST
  #define RP2040_SPI1_CONFIG_BAUDRATE                500 * 1000
  #define RP2040_SPI1_CONFIG_DATAWIDTH               8
  #if 0
  #endif
#endif
#if(RP2040_NUMBER_OF_SPI > 1)
  #define RP2040_SPI2_CONFIG_POLARITY                SPI_CPOL_0
  #define RP2040_SPI2_CONFIG_PHASE                   SPI_CPHA_0
  #define RP2040_SPI2_CONFIG_DIRECTION               SPI_MSB_FIRST
  #define RP2040_SPI2_CONFIG_BAUDRATE                500 * 1000
  #define RP2040_SPI2_CONFIG_DATAWIDTH               SPI_DATA_WIDTH
#endif

/**
 * I2C definitions
 */
#if BOARD_CONFIG_HAS_SECURE_ELEMENT
  #define LPC_NUMBER_OF_I2C                       1
#else
  #define RP2040_NUMBER_OF_I2C                       0
#endif

#if(RP2040_NUMBER_OF_I2C > 0)
#define RP2040_I2C1_TYPE                           I2C_INSTANCE_1 /* Secure Element */
//#define RP2040_I2C1_CLK_FRQ                        12000000
#define RP2040_I2C1_BAUDRATE                       100000
#define RP2040_I2C1_SLAVE_ADDR_SIZE                I2C_ADDR_SIZE_8
#endif
#if(RP2040_NUMBER_OF_I2C > 1)
#define RP2040_I2C2_TYPE                           I2C_INSTANCE_2 /* Free */
//#define RP2040_I2C2_CLK_FRQ                        12000000
#define RP2040_I2C2_BAUDRATE                       100000
#define RP2040_I2C2_SLAVE_ADDR_SIZE                I2C_ADDR_SIZE_8
#endif

/**
 * Radio definitions (LoRa Transceiver)
 */

#define RADIO_TCXO_WAKEUP_TIME      5
#define RADIO_RESET_PIN				RPIO_16  /* MCU pin 27 */
#define RADIO_ANT_SWITCH_PIN		RPIO_19  /* MCU pin 30 */
#define RADIO_BUSY_PIN				RPIO_17  /* MCU pin 28 */
#define RADIO_DIO_1_PIN				RPIO_18  /* MCU pin 29 */
#define RADIO_NSS_PIN				RPIO_13  /* MCU pin 16 */
#define RADIO_MOSI_PIN				RPIO_15  /* MCU pin 18 */
#define RADIO_MISO_PIN				RPIO_12  /* MCU pin 15 */
#define RADIO_SPI_CLK_PIN			RPIO_14  /* MCU pin 17 */
/* device selebt not available on tinyLoRa */
#define RADIO_DEVICE_SEL_PIN		NC

/**
 * LED pins
 */

#define LED_1     RPIO_25    /* MCU pin 37 */
/* LED 2 not available on tinyLoRa */
#define LED_2	  NC


/**
 * GNSS definitions
 */
#if BOARD_CONFIG_HAS_GNSS
  #define GNSS_UART_BAUDRATE				9600
  #define GNSS_PPS_PIN							PIO0_25
  #define GNSS_RESET_PIN						PIO0_22
#endif

/**
 * Power Mode Options
 */

/* TODO
 * add low power support for tinyLoRa */

#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__
