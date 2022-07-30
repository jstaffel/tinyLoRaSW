/*!
 * \file      uart-board.c
 *
 * \brief     Target board UART driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Diego Bienz
 * \author    Julian Staffelbach
 */

#include "board.h"
#include "uart-board.h"
#include "board-config.h"

/* pico specific libraries */
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define BOARD_DEFAULT_BAUDRATE	115200

/* hardware instances of uart on RP2040 */
#define UART_INSTANCE_1		((uart_inst_t *const)uart0_hw)
#define UART_INSTANCE_2		((uart_inst_t *const)uart1_hw)

typedef struct {
	UartId_t id;
	uart_inst_t *const uart_hw;
	uint32_t baudRate;
	uint8_t nOfDataBits;
	uint8_t nOfStopBits;
	uart_parity_t parity;
	PinNames rx_pin;
	PinNames tx_pin;
	bool ctsEnabled;
	bool rtsEnabled;
	const uint32_t irqn;
	const uint32_t interruptEnableRx;
	const uint32_t interruptEnableTx;
	void (*IrqNotify)(UartNotifyId_t id);
} RP2040UartHandle_t;

/**
  * Local Usart Handles
  * DO NOT CHANGE => If you need to change Usart settings, change it in board-config.h
  */

#if(RP2040_NUMBER_OF_USARTS > 0)
static RP2040UartHandle_t UsartHandle0 = {
	.id = UART_1,
	.uart_hw = UART_INSTANCE_1,
	.irqn = RP2040_USART1_IRQn,
	.interruptEnableRx = RP2040_USART1_IRQ_RX_ENABLE,
	.interruptEnableTx = RP2040_USART1_IRQ_TX_ENABLE
};
#endif
#if(RP2040_NUMBER_OF_USARTS > 1)
static RP2040UartHandle_t UsartHandle1 = {
	.id = UART_2,
	.uart_hw = UART_INSTANCE_2,
	.irqn = RP2040_USART2_IRQnRP2040_USART2_IRQn,
	.interruptEnableRx = RP2040_USART2_IRQ_RX_ENABLE,
	.interruptEnableTx = RP2040_USART2_IRQ_TX_ENABLE
};
#endif

static void MapUartIdToHandle(UartId_t uartId, RP2040UartHandle_t **handle);

/* each uart has a separate irq-handler */
void RP2040_UART1_IRQ_HANDLER(void);
void RP2040_UART2_IRQ_HANDLER(void);

/**
  * CAUTION:
  * The pin configuration (muxing, clock, etc.) is made with the pin_mux.* of the board.
  * You can also use the pin configuration tool of NXP.
  * This init function doesn't care about the handovered pins
  */
void UartMcuInit(Uart_t *obj, UartId_t uartId, PinNames tx, PinNames rx) {

	obj->UartId = uartId;

	RP2040UartHandle_t *handle;
	MapUartIdToHandle(uartId, &handle);

	#if(RP2040_NUMBER_OF_USARTS > 0)
	if(handle == &UsartHandle0){
		handle->baudRate = RP2040_USART1_BAUDRATE;
		handle->nOfDataBits = RP2040_USART1_DATA_BITS;
		handle->nOfStopBits = RP2040_USART1_STOP_BITS;
		handle->parity = RP2040_USART1_PARITY;
	}
#endif

#if(RP2040_NUMBER_OF_USARTS > 1)
	if(handle == &UsartHandle1){
		handle->baudRate = RP2040_USART2_BAUDRATE;
		handle->nOfDataBits = RP2040_USART2_DATA_BITS;
		handle->nOfStopBits = RP2040_USART2_STOP_BITS;
		handle->parity = RP2040_USART2_PARITY;
	}
#endif

	handle->tx_pin = tx;
	handle->rx_pin = rx;

	CRITICAL_SECTION_BEGIN();

	uart_init(handle->uart_hw, handle->baudRate);
	gpio_set_function(tx - RP2040_PINS_OFFSET, GPIO_FUNC_UART);
	gpio_set_function(rx - RP2040_PINS_OFFSET, GPIO_FUNC_UART);

	uart_set_format(handle->uart_hw, handle->nOfDataBits, handle->nOfStopBits, handle->parity);

	if(obj->IrqNotify != NULL){
		handle->IrqNotify = obj->IrqNotify;
		if(handle->id == UART_1){
			#if(RP2040_NUMBER_OF_USARTS > 0)
			irq_set_exclusive_handler(handle->irqn, RP2040_UART1_IRQ_HANDLER);
			#endif
		}else if(handle->id == UART_2){
			#if(RP2040_NUMBER_OF_USARTS > 1)
			irq_set_exclusive_handler(handle->irqn, RP2040_UART2_IRQ_HANDLER);
			#endif
		}
		irq_set_enabled(handle->irqn, true);
		uart_set_irq_enables(handle->uart_hw, handle->interruptEnableRx, handle->interruptEnableTx);
	}

	CRITICAL_SECTION_END();
}

void UartMcuConfig(Uart_t *obj, UartMode_t mode, uint32_t baudrate,
		WordLength_t wordLength, StopBits_t stopBits, Parity_t parity,
		FlowCtrl_t flowCtrl) {

	RP2040UartHandle_t *handle;

	MapUartIdToHandle(obj->UartId, &handle);

	handle->baudRate = baudrate;

	if(wordLength == UART_8_BIT){
		handle->nOfDataBits = 8;
	} else if(wordLength == UART_9_BIT){
		handle->nOfDataBits = 9;
	}
	
	switch (stopBits)
	{
	case UART_1_STOP_BIT: handle->nOfStopBits = 1;
		break;
	case UART_2_STOP_BIT: handle->nOfStopBits = 2;
		break;
	default: handle->nOfStopBits = 1;
		break;
	}

	if(parity == NO_PARITY){
		handle->parity = UART_PARITY_NONE;
	}else if(parity == EVEN_PARITY){
		handle->parity = UART_PARITY_EVEN;
	}else{
		handle->parity = UART_PARITY_ODD;
	}

	if(flowCtrl == NO_FLOW_CTRL){
		handle->ctsEnabled = false;
		handle->rtsEnabled = false;
	}else if(flowCtrl == RTS_FLOW_CTRL){
		handle->ctsEnabled = false;
		handle->rtsEnabled = true;
	}else if(flowCtrl == CTS_FLOW_CTRL){
		handle->ctsEnabled = true;
		handle->rtsEnabled = false;
	}else if(flowCtrl == RTS_CTS_FLOW_CTRL){
		handle->ctsEnabled = true;
		handle->rtsEnabled = true;
	}

	CRITICAL_SECTION_BEGIN();

	uart_init(handle->uart_hw, handle->baudRate);
	gpio_set_function(handle->tx_pin - RP2040_PINS_OFFSET, GPIO_FUNC_UART);
	gpio_set_function(handle->rx_pin - RP2040_PINS_OFFSET, GPIO_FUNC_UART);

	uart_set_format(handle->uart_hw, handle->nOfDataBits, handle->nOfStopBits, handle->parity);
	uart_set_hw_flow(handle->uart_hw, handle->ctsEnabled, handle->rtsEnabled);

	if(obj->IrqNotify != NULL){
		handle->IrqNotify = obj->IrqNotify;
		if(handle->id == UART_1){
			#if(RP2040_NUMBER_OF_USARTS > 0)
			irq_set_exclusive_handler(handle->irqn, RP2040_UART1_IRQ_HANDLER);
			#endif
		}else if(handle->id == UART_2){
			#if(RP2040_NUMBER_OF_USARTS > 1)
			irq_set_exclusive_handler(handle->irqn, RP2040_UART2_IRQ_HANDLER);
			#endif
		}
		irq_set_enabled(handle->irqn, true);
		uart_set_irq_enables(handle->uart_hw, handle->interruptEnableRx, handle->interruptEnableTx);
	}

	CRITICAL_SECTION_END();
}

void UartMcuDeInit(Uart_t *obj) {

	RP2040UartHandle_t *handle;
	MapUartIdToHandle(obj->UartId, &handle);

	/* Disable interrupts if there is a callback function */
	if (handle->IrqNotify != NULL) {
		irq_set_enabled(handle->irqn, true);
		uart_set_irq_enables(handle->uart_hw, false, false);
	}
	
	uart_deinit(handle->uart_hw);
}

uint8_t UartMcuPutChar(Uart_t *obj, uint8_t data) {

	RP2040UartHandle_t *handle;
	MapUartIdToHandle(obj->UartId, &handle);

	if(!uart_is_writable(handle->uart_hw)){
		return 1;
	}

	uart_putc_raw(handle->uart_hw, (char)data);
	return 0;
}

uint8_t UartMcuGetChar(Uart_t *obj, uint8_t *data) {
	
	RP2040UartHandle_t *handle;
	MapUartIdToHandle(obj->UartId, &handle);

	if(!uart_is_readable(handle->uart_hw)){
		return 1;
	}

	*data = uart_getc(handle->uart_hw);
	return 0;
}

uint8_t UartMcuPutBuffer(Uart_t *obj, uint8_t *buffer, uint16_t size) {
	
	RP2040UartHandle_t *handle;
	MapUartIdToHandle(obj->UartId, &handle);

	if(!uart_is_writable(handle->uart_hw)){
		return 1;
	}

	uart_write_blocking(handle->uart_hw, buffer, size);
	return 0;
}

uint8_t UartMcuGetBuffer(Uart_t *obj, uint8_t *buffer, uint16_t size,
		uint16_t *nbReadBytes) {

	RP2040UartHandle_t *handle;
	MapUartIdToHandle(obj->UartId, &handle);

	if(!uart_is_readable(handle->uart_hw)){
		return 1;
	}

	uart_read_blocking(handle->uart_hw, buffer, size);
	*nbReadBytes = sizeof(buffer) / sizeof(buffer[0]);
	return 0;
}

/**
  * Since the LoRaMac-node stack's Uart enum (UartId_t) goes from Uart1 to Uart2,
  * but the LPC series offers several Usarts on different pin settings, a short mapping
  * is required. This method sets the pointer "handle" to the corresponding handle of the given
  * uartId defined above.
  */
static void MapUartIdToHandle(UartId_t uartId, RP2040UartHandle_t **handle) {
	
	#if(RP2040_NUMBER_OF_USARTS > 0)
	if(UsartHandle0.id == uartId){
		*handle = &UsartHandle0;
	}
#endif
#if(RP2040_NUMBER_OF_USARTS > 1)
	if(UsartHandle1.id == uartId){
		*handle = &UsartHandle1;
	}
#endif
}

#if(RP2040_NUMBER_OF_USARTS > 0)
void RP2040_UART1_IRQ_HANDLER(void) {

	if(uart_is_readable(UsartHandle0.uart_hw)){
		UsartHandle0.IrqNotify(UART_NOTIFY_RX);
	}else if(uart_is_writable(UsartHandle0.uart_hw)){
		UsartHandle0.IrqNotify(UART_NOTIFY_TX);
	}
}
#endif

#if(RP2040_NUMBER_OF_USARTS > 1)
void RP2040_UART2_IRQ_HANDLER(void) {

	if(uart_is_readable(UsartHandle1.uart_hw)){
		UsartHandle1.IrqNotify(UART_NOTIFY_RX);
	}else if(uart_is_writable(UsartHandle1.uart_hw)){
		UsartHandle1.IrqNotify(UART_NOTIFY_TX);
	}
}
#endif
