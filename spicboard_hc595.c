#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "avr_ioport.h"
#include "avr_spi.h"

#include "spicboard_hc595.h"
#include "spicboard.h"
#include "sim_avr.h"

enum {
	IRQ_BYTE_IN,
	IRQ_LATCH,
	IRQ_COUNT
};

static const char * irq_names[IRQ_COUNT] = {
	[IRQ_BYTE_IN] = "74HC595.bytein",
	[IRQ_LATCH] = "74HC595.latch",
};

static avr_irq_t * irq;
static uint8_t spi_byte = 0;

static void hc595_byte_in(struct avr_irq_t * irq, uint32_t value, void * param) {
	spi_byte = (value & 0xff);
}

static void hc595_latch(struct avr_irq_t * irq, uint32_t value, void * param) {
	if (irq->value && !value){
		unsigned active = (spi_byte >> 7) & 1;
		for (unsigned i = 0; i < 7; i++) {
			led_set((active ? LED_7SEG_1_0 : LED_7SEG_0_0) + i, ~(spi_byte >> i) & 1);
			led_set((active ? LED_7SEG_0_0 : LED_7SEG_1_0) + i, false);
		}
	}
}

void hc595_init() {
	irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_COUNT, irq_names);

	avr_irq_t * i_mosi = avr_io_getirq(avr, AVR_IOCTL_SPI_GETIRQ('0'), SPI_IRQ_OUTPUT),
	          * i_latch = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2);

	avr_connect_irq(i_mosi, irq + IRQ_BYTE_IN);
	avr_connect_irq(i_latch, irq + IRQ_LATCH);

	avr_irq_register_notify(irq + IRQ_BYTE_IN, hc595_byte_in, NULL);
	avr_irq_register_notify(irq + IRQ_LATCH, hc595_latch, NULL);
}

