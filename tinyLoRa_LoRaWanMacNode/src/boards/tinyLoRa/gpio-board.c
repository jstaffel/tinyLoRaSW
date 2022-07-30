/*!
 * \file      gpio-board.c
 *
 * \brief     Target board GPIO driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include <stdbool.h>
#include "gpio-board.h"
#include "utilities.h"
#include "board-config.h"

/* pico specific libraries */
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/**
  * Interrupt definitions, 29 gpio pins available
  */
#define BOARD_NUMBER_OF_INTERRUPT_INPUTS			29

/**
  * Array that holds all pin-handles, that have an irq-callbak attached
  * In GpioIrqCallback the array is used to determine if a pin has an irq and the pointer to it is stored in the handler.
  */
static Gpio_t *interruptPins[BOARD_NUMBER_OF_INTERRUPT_INPUTS];

/**
  * Callback function for all interrupts from PINT
  */
void GpioIrqCallback(uint gpio_pin, uint32_t events);

/**
  * CAUTION:
  * The pin configurations like muxing, clock, etc. is made with the pin_mux.* of the board.
  * You can also use the pin configuration tool of NXP.
  * This init function doesn't care about the handovered pin configurations
  */
void GpioMcuInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value) {
	
	uint8_t pin_direction;

	if(pin >= RPIO_0){
		obj->pin = pin;
		if(config == PIN_PUSH_PULL){
			obj->pull = type;
		}

		/* RP2040-Pins is a list that starts at (uint) 192 and ends at (uint) 221,
		with the according value are stored in obj->pin.
		therefore, -192 has to be subtracted from each pin
		obj->pinIndex gets mapped to a number between 0 and 29 */
		obj->pinIndex = (obj->pin - RP2040_PINS_OFFSET);

		if(obj->pinIndex < 0 || obj->pinIndex > 29){
			while(true){}	/* invalid pin */
		}

		switch(mode) {
		  case PIN_INPUT:
		    pin_direction = GPIO_IN;
		    break;
		  case PIN_OUTPUT:
        	pin_direction = GPIO_OUT;
        	break;
		  default:
		  case PIN_ALTERNATE_FCT:
		  case PIN_ANALOGIC:
		    /* SDK does not support these. Using input instead */
        	pin_direction = GPIO_IN;
        break;
		}

		gpio_init(obj->pinIndex);
		gpio_set_dir(obj->pinIndex, pin_direction);
		if(pin_direction == GPIO_IN){
			gpio_set_input_enabled(obj->pinIndex, true);
		}

		if(obj->pull == PIN_PULL_UP){
			gpio_pull_up(obj->pinIndex);
		}else if(obj->pull == PIN_PULL_DOWN){
			gpio_pull_down(obj->pinIndex);
		} else {
			gpio_disable_pulls(obj->pinIndex);
		}

		if(mode == PIN_OUTPUT){
			gpio_put(obj->pinIndex, value);
		}

	} else if (pin == NC){
		return;
	}
}

void GpioMcuSetContext(Gpio_t *obj, void *context) {
	obj->Context = context;
}

/**
  * CAUTION: GPIO has to be initialised before, independent
  * whether as input or output
  */
void GpioMcuSetInterrupt(Gpio_t *obj, IrqModes irqMode,
		IrqPriorities irqPriority, GpioIrqHandler *irqHandler) {

	int irqTrigger;

	if(obj->pin >= RPIO_0){

		if(irqHandler == NULL){
			return;
		}

		obj->IrqHandler = irqHandler;

		if( irqMode == IRQ_RISING_EDGE )
        {
            irqTrigger = GPIO_IRQ_EDGE_RISE;
        }
        else if( irqMode == IRQ_FALLING_EDGE )
        {
            irqTrigger = GPIO_IRQ_EDGE_FALL;
        }
        else
        {
            irqTrigger = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
        }

		interruptPins[ (obj->pin) - RP2040_PINS_OFFSET] = obj;
		gpio_set_irq_enabled_with_callback(obj->pinIndex, irqTrigger, true, &GpioIrqCallback);
	}
}

void GpioMcuRemoveInterrupt(Gpio_t *obj) {

	if(obj->pin >= RPIO_0){
		interruptPins[ (obj->pin) - RP2040_PINS_OFFSET]->IrqHandler = NULL;
		interruptPins[ (obj->pin) - RP2040_PINS_OFFSET] = NULL;
	}
}

void GpioMcuWrite(Gpio_t *obj, uint32_t value) {

	if (obj == NULL) {
    	while(true) {
      		/* You should not reach this state. Invalid pin */
    	}
	}

	if (obj->pin >= RPIO_0) {
		if (obj->pin == NC) {
			return;
		}
		gpio_put(obj->pinIndex, value);
	}
}

void GpioMcuToggle(Gpio_t *obj) {

	if (obj == NULL) {
    	while(true) {
      		/* You should not reach this state. Invalid pin */
    	}
	}

	if (obj->pin >= RPIO_0) {
		if (obj->pin == NC) {
			return;
		}
		gpio_put(obj->pinIndex, !gpio_get(obj->pinIndex));
	}
}

uint32_t GpioMcuRead(Gpio_t *obj) {
	  
	if (obj == NULL) {
    	while(true) {
      		/* You should not reach this state. Invalid pin */
    	}
  	}

	if (obj->pin >= RPIO_0) {
		if (obj->pin == NC) {
			return -1;
		}
		return gpio_get(obj->pinIndex);
	}

	return -1;
}

void GpioIrqCallback(uint gpio_pin, uint32_t events){
	if(interruptPins[gpio_pin] != NULL && interruptPins[gpio_pin]->IrqHandler != NULL){
		interruptPins[gpio_pin]->IrqHandler(interruptPins[gpio_pin]->Context);
	}
}