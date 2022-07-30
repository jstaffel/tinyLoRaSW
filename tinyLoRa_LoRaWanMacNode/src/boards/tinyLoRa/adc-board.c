/*!
 * \file      adc-board.c
 *
 * \brief     Target board ADC driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include "board-config.h"
#include "adc-board.h"

/* pico specific libraries */
#include "hardware/adc.h"

void AdcMcuInit( Adc_t *obj, PinNames adcInput )
{   
    adc_init();
    
    /* there are four adc-pins available on tinyLoRa, GPIO26 - GPIO29 */
    if(adcInput >= RPIO_26 && adcInput <= RPIO_29){
        obj->AdcInput.pin = adcInput;
        obj->AdcInput.pinIndex = adcInput - RP2040_PINS_OFFSET;
        adc_gpio_init(obj->AdcInput.pinIndex);
    }
}

void AdcMcuConfig( void )
{
    /* Everything done in AdcMcuInit */
}

uint16_t AdcMcuReadChannel( Adc_t *obj, uint32_t channel )
{
    /* input is: 0 for GPIO26, 1 for GPIO27, 2 for GPIO28, 3 for GPIO29 */
    adc_select_input(obj->AdcInput.pinIndex - 26);
    return adc_read();
}