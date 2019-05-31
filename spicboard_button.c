#include "spicboard_button.h"

#include "avr_ioport.h"

static avr_irq_t * irq;

static const char * irq_names[BUTTONS] = {
	[BUTTON0] = "BTN0",
	[BUTTON1] = "BTN1",
};

void button_set(enum BUTTON btn, enum BUTTONSTATE state){
	avr_raise_irq(irq + btn, state);
}

void button_init(struct avr_t * avr){
	// Buttons
	irq = avr_alloc_irq(&avr->irq_pool, 0, BUTTONS, irq_names);
	avr_connect_irq(irq + BUTTON0, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
	avr_connect_irq(irq + BUTTON1, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
}
