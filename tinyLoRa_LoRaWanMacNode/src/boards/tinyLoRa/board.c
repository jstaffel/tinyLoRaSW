/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include <stdio.h>
#include "utilities.h"
#include "sx-uart.h"
#include "board-config.h"
#include "board.h"
#include "sx126x-board.h"
#include "sx-gpio.h"
#include "rtc-board.h"
#include "sx-spi.h"
#include "sx-delay.h"
#include "sx-gps.h"
#include "sx-i2c.h"
#include "i2c-board.h"
#include "lpm-board.h"
#include "eeprom-board.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "hardware/flash.h"
#include "hardware/clocks.h"

#include "RP2040-platform.h"

/*!
 * Uart objects
 */
#if(LPC_NUMBER_OF_USARTS > 0)
Uart_t Uart0;  // Board Uart
#endif

#if BOARD_CONFIG_HAS_GNSS
Uart_t Uart1;  // GPS
#endif

#if PL_CONFIG_USE_LED1
Gpio_t Led1;
#endif
#if PL_CONFIG_USE_LED2
Gpio_t Led2;
#endif
#if PL_CONFIG_USE_TESTPIN3
Gpio_t testGpio3;
#endif

#if BOARD_CONFIG_HAS_SECURE_ELEMENT
/*!
 * I2c objects
 */
I2c_t I2c0;  // Secure Element

/*!
 * Puts the secure element into sleep mode
 */
static void BoardPutSecureElementInSleepMode( void );
#endif

/*!
 * Puts the radio in sleep mode
 * If coldstart enabled, the radio will loose it's configuration
 * and needs to be reinitialised after wake up
 */
static void BoardPutRadioInSleepMode(bool coldstart);

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * System Clock Configuration
 */
static void SystemClockConfig( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );

/*!
 * \brief Initializes the EEPROM emulation driver to access the flash.
 *
 * \remark This function is defined in eeprom-board.c file
 */
void EepromMcuInit( void );

/*!
 * \brief Indicates if an erasing operation is on going.
 *
 * \remark This function is defined in eeprom-board.c file
 *
 * \retval isEradingOnGoing Returns true is an erasing operation is on going.
 */
bool EepromMcuIsErasingOnGoing( void );

void BoardCriticalSectionBegin( uint32_t *mask )
{
    *mask = save_and_disable_interrupts();
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    restore_interrupts(*mask);
}

void BoardInitPeriph( void )
{
#if BOARD_CONFIG_HAS_GNSS
	  GpsInit();
#endif
}

void BoardInitMcu( void )
{ 
    clocks_init();
    stdio_init_all();

#if(LPC_NUMBER_OF_USARTS > 0)
	// Configure your terminal for 8 Bits data (7 data bit + 1 parity bit), no parity and no flow ctrl
	UartInit( &Uart0, UART_1, NC, NC );
	UartConfig( &Uart0, RX_TX, BOARD_UART_BAUDRATE, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
#endif

  // LEDs
#if PL_CONFIG_USE_LED1
  GpioInit(&Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
#endif
#if PL_CONFIG_USE_LED2
  GpioInit(&Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
#endif
#if PL_CONFIG_USE_TESTPIN3
  GpioInit(&testGpio3, RPIO_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

	RtcInit( );

	BoardUnusedIoInit( );

  //SPI for LoRa transceiver
  SpiInit(&SX126x.Spi, SPI_1, RADIO_MOSI_PIN, RADIO_MISO_PIN, RADIO_SPI_CLK_PIN, NC);
	SX126xIoInit( );
	SX126xIoDbgInit();
	SX126xReset();
	SX126xIoTcxoInit();

#if BOARD_CONFIG_HAS_SECURE_ELEMENT
	//I2C for Secure Element
  I2cInit(&I2c0, I2C_1, NC, NC);
#endif

  //Actually flash on this platform
   EepromMcuInit();

}

void BoardResetMcu( void )
{ 
  /* TODO
   * add reset support for tinyLoRa */
}

void BoardDeInitMcu( void )
{ 

	GpsStop();
#if(LPC_NUMBER_OF_USARTS > 0)
	if(Uart0.IsInitialized) {
		UartDeInit(&Uart0);
	}
#endif
#if BOARD_CONFIG_HAS_GNSS
	if(Uart1.IsInitialized) {
		UartDeInit(&Uart1);
	}
#endif

	BoardPutRadioInSleepMode(true);
	SpiDeInit(&SX126x.Spi);
#if	BOARD_CONFIG_HAS_SECURE_ELEMENT
	BoardPutSecureElementInSleepMode();
	I2cDeInit(&I2c0);
#endif

}

void BoardGetUniqueId(uint8_t *id)
{
  flash_get_unique_id(id);
}

uint32_t BoardGetRandomSeed( void )
{ 
	uint32_t seed[4];
  uint8_t flash_id[8];

  BoardGetUniqueId(flash_id);

	seed[0] = 0;
	for(int i = 0; i < 4; i++){
		seed[0] += flash_id[i] << 8*i;
	}
	seed[1] = 0;
	for(int i = 0; i < 4; i++){
		seed[1] += flash_id[i+1] << 8*i;
	} 
	seed[2] = 0;
	for(int i = 0; i < 4; i++){
		seed[2] += flash_id[i+2] << 8*i;
	}
	seed[3] = 0;
	for(int i = 0; i < 4; i++){
		seed[3] += flash_id[i+3] << 8*i;
	}
  return seed[0] ^ seed[1] ^ seed[2] ^ seed[3];
}

/**
  * NOT IMPLEMENTED ON THIS PLATFORM
  */
uint8_t BoardGetBatteryLevel( void )
{
    return 0;
}

static void BoardUnusedIoInit( void )
{
    /* Nothing to do */
}

void SystemClockConfig( void )
{ 
  clocks_init();
}

void SystemClockReConfig( void )
{ 
    clocks_init();
}

#if !McuLib_CONFIG_SDK_USE_FREERTOS
void SysTick_Handler( void )
{
	/* SysTick interrupt. Could be used to react on that */
}
#endif

/*!
 * \brief Enters off Power Mode (Deep Power Down on LPC55)
 * A reset is executed automatically on wake up
 *
 * Deep power-down: Deep-power down mode shuts down virtually all on-chip power
 *	consumption but requires a significantly longer wake-up time (compared to
 *	power-down mode). For maximal power savings, the entire system (CPU and all
 *	peripherals) is shut down except for the PMU, the PMC, the RTC and the OS event
 *	timer. On wake-up, the part reboots.
 *
 * IMPORTANT: SRAM retention defines which RAM sections should be retained while deep power down. It is important to place
 * variables in the retained sections if they are used after wake up from deep power down. One can place a variable
 * in a specified section as follows:
 *
 * bool __attribute__((section (".retainedSection"))) myBool = false;
 *
 * While the defined section .retainedSection is defined in the linker script.
 */
void LpmEnterOffMode( void ){
	/* TODO */
}

/*!
 * \brief Exits off Power Mode
 */
void LpmExitOffMode( void ){
	/* TODO */
}

/**
  * \brief Enters Low Power Stop Mode
  *
  * \note ARM exits the function when waking up
  */
void LpmEnterStopMode( void)
{ 
  /* TODO */
}

/*!
 * \brief Exits Low Power Stop Mode
 */
void LpmExitStopMode( void )
{ 
  /* TODO */
}

/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode( void)
{ 
  /* TODO */
}

void BoardLowPowerHandler( void )
{ 
    /* low power mode not supported in tinyLoRa yet */
    #if 0
    // Wait for any cleanup to complete before entering standby/shutdown mode
    while( EepromMcuIsErasingOnGoing( ) == true ){ }

    CRITICAL_SECTION_BEGIN();
    
    /*!
     * If an interrupt has occurred after __disable_irq( ), it is kept pending
     * and cortex will not enter low power anyway
     */

    LpmEnterLowPower( );

    CRITICAL_SECTION_END();
    #endif
}

static void BoardPutRadioInSleepMode(bool coldstart){
    SleepParams_t params = { 0 };
    params.Fields.WarmStart = coldstart;

    SX126xSetSleep( params );
}

void BoardPrintUUID(void) {
  uint8_t uid[8];
  unsigned char buf[96];

  BoardGetUniqueId(&uid);

  printf("Board Unique ID: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);
}

#if BOARD_CONFIG_HAS_SECURE_ELEMENT
static void BoardPutSecureElementInSleepMode( void ){
    /* Wake up */
	uint8_t buffer[1] = {0};
    I2cMcuWriteBuffer(&I2c0, 0x00, 0, buffer, (size_t)0);
	DelayMs(100);

	/* Send to sleep */
    buffer[0] = 0x1;  // sleep word address value
    I2cMcuWriteBuffer(&I2c0, 0xC0, 0x1, buffer, (size_t)1);
}
#endif

#if !defined ( __CC_ARM )

/*
 * Function to be used by stdout for printf etc
 */
/* done in Pico-SDK */
#if 0
int _write( int fd, const void *buf, size_t count )
{
}
#endif

/*
 * Function to be used by stdin for scanf etc
 */
/* done in Pico-SDK */
#if 0
int _read( int fd, const void *buf, size_t count )
{ 
}
#endif

#else

#include <stdio.h>

// Keil compiler
int fputc( int c, FILE *stream )
{
    while( UartPutChar( &Uart0, ( uint8_t )c ) != 0 );
    return c;
}

int fgetc( FILE *stream )
{
    uint8_t c = 0;
    while( UartGetChar( &Uart0, &c ) != 0 );
    // Echo back the character
    while( UartPutChar( &Uart0, c ) != 0 );
    return ( int )c;
}

#endif

#ifdef USE_FULL_ASSERT

#include <stdio.h>

/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\n", file, line) */

    printf( "Wrong parameters value: file %s on line %lu\n", ( const char* )file, line );
    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
