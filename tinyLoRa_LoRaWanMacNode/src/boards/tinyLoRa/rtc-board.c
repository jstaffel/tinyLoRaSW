/*!
 * \file      rtc-board.c
 *
 * \brief     Target board RTC timer
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include "board.h"
#include "eeprom-board.h"
#include "board-config.h"
#include "rtc-board.h"
#include "lpm-board.h"
#include "sx-timer.h"
#include "sx-gps.h"
#include "utilities.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "pico/time.h"

/* address-offset of flash to write rtc data */
#define BACKUP_FLASH_OFFSET 14 * 4096

/* init time */
static datetime_t epochTime;

/*!
 * RTC timer context
 */
typedef struct {
	uint32_t Time;  /* Reference time */
} RtcTimerContext_t;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the RtcSetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
static RtcTimerContext_t RtcTimerContext;

/*!
 * State if an alarm is pending
 */
static bool PendingAlarm = false;

/*!
 * External Nmea GPS data, stored in gps.c
 */
extern NmeaGpsData_t NmeaGpsData;

/*!
 * Function to check if a year is a leap year
 */
bool yearIsLeapYear(uint32_t year);

/*!
 * Callback function for the HW-Timer
 */
int64_t TimerCallback(alarm_id_t id, void *user_data);

/*!
 * Callback function for the OS Timer
 */
void RtcOSTimerCallback(void);

void RtcInit(void) {

#if McuLib_CONFIG_SDK_USE_FREERTOS
  	// NVIC_SetPriority(OS_EVENT_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
#endif

	datetime_t initDate;
	initDate.year = 2020U;
	initDate.month = 12U;
	initDate.day = 12U;
	initDate.hour = 12U;
	initDate.min = 0;
	initDate.sec= 0;
	initDate.dotw = 6;
	epochTime = initDate;

	/* RTC initialization and start */
	rtc_init();
	rtc_set_datetime(&initDate);

#if McuLib_CONFIG_SDK_USE_FREERTOS
  NVIC_SetPriority(RTC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
#endif
}

uint32_t RtcGetMinimumTimeout(void) {
	/* Minimum is 1 ms */
	return RtcMs2Tick(1);
}

uint32_t RtcMs2Tick(TimerTime_t milliseconds) {
	/* tick frequency is 1 MHz (1 us counter is used) */
	return (uint32_t)milliseconds * 1000;
}

TimerTime_t RtcTick2Ms(uint32_t tick) {
	/* tick frequency is 1 MHz (1 us counter is used) */
	return (TimerTime_t)tick / 1000;
}

void RtcDelayMs(TimerTime_t milliseconds) {	
	busy_wait_ms((uint32_t)milliseconds);
}

void RtcSetAlarm(uint32_t timeout) {
	RtcStartAlarm(timeout);
}

void RtcStopAlarm(void) {
	PendingAlarm = false;
}

void RtcStartAlarm(uint32_t timeout) {

	/* workaround for just now, needs to be fixed 
	 * when using ping-pong on tinyLoRa, use this workaround for now 
	 */
	if(timeout == 3000000){
		return;
	}

	add_alarm_in_us((uint64_t)timeout, TimerCallback, NULL, false);
	RtcStopAlarm();
	PendingAlarm = true;
}

uint32_t RtcSetTimerContext(void) {

	RtcTimerContext.Time = RtcGetTimerValue();
	return RtcTimerContext.Time;
}

uint32_t RtcGetTimerContext(void) {
	return RtcTimerContext.Time;
}

uint32_t RtcGetCalendarTime(uint16_t *milliseconds) {
	*milliseconds = 0;

	/* implementation based on https://www.experts-exchange.com/questions/26868405/convert-unix-time-stamp-for-8-bit-C-compiler.html
	 * calculates the numbers of seconds since the application was first started */
  	uint8_t daysmonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  	int32_t nofSecs = 0;
  	uint16_t year;
  	uint8_t month;
	datetime_t currentTime;
	rtc_get_datetime(&currentTime);

  	/* calculate the seconds for the given day */
  	nofSecs = (int32_t)(currentTime.sec - epochTime.sec);
  	nofSecs += (int32_t)(currentTime.min * 60 - epochTime.min * 60);
  	nofSecs += (int32_t)(currentTime.hour * 3600 - epochTime.hour * 3600);
  	nofSecs += (int32_t)((currentTime.day - 1) * 24 * 3600 - (epochTime.day - 1) * 24 * 3600);

  	/* month update based on leap year */
  	if (yearIsLeapYear(currentTime.year)) { /* adjust leap year month days */
  	  	daysmonth[2] = 29;
  	} else {
    	daysmonth[2] = 28;
  	}
  	/* calculate the seconds for the months in the given date */
  	for (month = currentTime.month-1; month > epochTime.month; month--) {
    	nofSecs += (int32_t)daysmonth[month]*24*3600;
  	}

  	/* count the years back to one year adter the epoch year */
  	for (year=currentTime.year-1; year > epochTime.year; year--) {
    	if (yearIsLeapYear(year)) {
      		nofSecs += (365+1)*24*3600;
    	} else {
      		nofSecs += 365*24*3600;
    	}
  	}

	if (yearIsLeapYear(epochTime.year)) { /* adjust leap year month days */
  	  	daysmonth[2] = 29;
  	} else {
    	daysmonth[2] = 28;
  	}

	/* seconds off all days left in the month except current day */
	for(month = 12; month > epochTime.month; month--){
		nofSecs += (int32_t)daysmonth[month] * 24 * 3600; 
	}

	rtc_get_datetime(&currentTime);
	uint8_t sec1 = currentTime.sec;
	uint8_t sec2 = sec1;

	/* wait for a new second to start and return the increased value immediately */
	while(sec1 == sec2){
		rtc_get_datetime(&currentTime);
		sec2 = currentTime.sec;
	}

	nofSecs++;
  	return nofSecs;
}

uint32_t RtcGetTimerValue(void) {
	return time_us_32();
}

uint32_t RtcGetTimerElapsedTime(void) {
	return (uint32_t)(RtcGetTimerValue() - RtcTimerContext.Time);
}

/*!
 * Data is written to the predefined address BACKUP_FLASH_ADDRESS in the flash
 */
void RtcBkupWrite(uint32_t data0, uint32_t data1) {

	uint8_t flashBackupPage[2*sizeof(uint32_t)];

	/* Save data0 to the first 4 bytes (Big endian) */
	flashBackupPage[0] = data0 >> 24;
	flashBackupPage[1] = data0 >> 16;
	flashBackupPage[2] = data0 >>  8;
	flashBackupPage[3] = data0;

	/* Save data1 to the second 4 bytes (Big endian) */
	flashBackupPage[4] = data1 >> 24;
	flashBackupPage[5] = data1 >> 16;
	flashBackupPage[6] = data1 >>  8;
	flashBackupPage[7] = data1;

	EepromMcuWriteBuffer(BACKUP_FLASH_OFFSET, flashBackupPage, sizeof(flashBackupPage));
}

/*!
 * Data is read from the predefined address BACKUP_FLASH_ADDRESS in the flash
 * CAUTION: If the data was not written to this address before, the result is unpredictable
 */
void RtcBkupRead(uint32_t *data0, uint32_t *data1) {

	uint8_t flashBackupPage[2*sizeof(uint32_t)];
	LmnStatus_t status;

	*data0 = 0;
	*data1 = 0;
	status = EepromMcuReadBuffer(BACKUP_FLASH_OFFSET, flashBackupPage, sizeof(flashBackupPage));
	if(status == LMN_STATUS_OK){
		*data0 = flashBackupPage[0];
		*data0 = (*data0  << 8) + flashBackupPage[1];
		*data0 = (*data0  << 8) + flashBackupPage[2];
		*data0 = (*data0  << 8) + flashBackupPage[3];

		*data1 = flashBackupPage[4];
		*data1 = (*data1  << 8) + flashBackupPage[5];
		*data1 = (*data1  << 8) + flashBackupPage[6];
		*data1 = (*data1  << 8) + flashBackupPage[7];
	}
}

void RtcProcess(void) {
	/* Not used on this board */
}

TimerTime_t RtcTempCompensation(TimerTime_t period, float temperature) {
	return period;
}

bool yearIsLeapYear(uint32_t year){
	/* implementation from Erich Styger's Library, Function: IsLeapYear
	 * https://github.com/ErichStyger/McuOnEclipseLibrary/blob/master/lib/src/McuUtility.c
	*/
	return ((((year%4)==0) && (year%100)!=0) || (year%400)==0);
}

int64_t TimerCallback(alarm_id_t id, void *user_data){
	RtcOSTimerCallback();
	return 0;
}

void RtcOSTimerCallback(void) {
	if(PendingAlarm){
		RtcStopAlarm();
		TimerIrqHandler();
	}
}
