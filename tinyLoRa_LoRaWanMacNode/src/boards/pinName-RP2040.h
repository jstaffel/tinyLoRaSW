/*!
 * \file      pinName-lpc.h
 *
 * \brief     Target board LPC pins definitions
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */
#ifndef __PIN_NAME_RP2040_H__
#define __PIN_NAME_RP2040_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
  *  RP2040 GPIO Mask Definitions, 0x1D = 29, amount of GPIO-Pins
  */
#define RP2040_PINS_OFFSET		192

#define RP2040_PINS \
    RPIO_0 = 192, RPIO_1, RPIO_2, RPIO_3, RPIO_4, RPIO_5, RPIO_6, RPIO_7, RPIO_8, RPIO_9, \
    RPIO_10, RPIO_11, RPIO_12, RPIO_13, RPIO_14, RPIO_15, RPIO_16, RPIO_17, RPIO_18, RPIO_19, \
    RPIO_20, RPIO_21, RPIO_22, RPIO_23, RPIO_24, RPIO_25, RPIO_26, RPIO_27, RPIO_28, RPIO_29  
#ifdef __cplusplus
}
#endif

#endif // __PIN_NAME_RP2040_H__
