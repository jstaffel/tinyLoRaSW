/*!
 * \file      delay-board.c
 *
 * \brief     Target board delay implementation
 * 
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz (HSLU)
 * \author    Erich Styger (HSLU)
 * \author    Julian Staffelbach (HSLU)
 */

#include "board.h"
#include "delay-board.h"

/* pico specific libraries */
#include "pico/stdlib.h"

void DelayMsMcu(uint32_t ms) {
#if McuLib_CONFIG_SDK_USE_FREERTOS
  if (xTaskGetSchedulerState()!=taskSCHEDULER_RUNNING) {
    McuWait_Waitms(ms); /* if scheduler is not yet running */
  } else { /* use RTOS wait */
    TickType_t xTicks = pdMS_TO_TICKS(ms);
    if (xTicks==0) {
      xTicks = 1; /* wait at least one tick */
    }
    vTaskDelay(xTicks);
  }
#else
	busy_wait_ms(ms);
#endif
}
