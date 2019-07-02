#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "spicboard_button.h"
#include "spicboard.h"

#include "sim_avr.h"
#include "avr_ioport.h"

static avr_irq_t irq[BUTTONS];
static avr_ioport_external_t ext = { .mask = (1 << 2) | (1 << 3), .value = 0xff } ;

static const char * irq_names[BUTTONS] = {
	[BUTTON0] = "BTN0",
	[BUTTON1] = "BTN1",
	[BUTTON_USER] = "BTNUSER",
};

static bool update = false;
static enum BUTTONSTATE btnState[BUTTONS];

void button_set(enum BUTTON btn, enum BUTTONSTATE state){
	if (state != BUTTON_UNCHANGED) {
		spicboard.btn[btn] = state;
		__atomic_store_n(btnState + btn, state, __ATOMIC_RELAXED);
		__atomic_store_n(&update, true, __ATOMIC_RELAXED);
	}
}

void button_raise_irq() {
	if (__atomic_load_n(&update, __ATOMIC_RELAXED)) {
		__atomic_store_n(&update, false, __ATOMIC_RELAXED);
		for (int i = 0 ; i < BUTTONS; i++) {
			enum BUTTONSTATE s = __atomic_exchange_n(btnState + i, BUTTON_UNCHANGED, __ATOMIC_RELAXED);
			if (s != BUTTON_UNCHANGED) {
				if (i == BUTTON_USER) {
					avr_raise_irq(irq + i, s);
				} else { // Hack.
					if (s == BUTTON_PRESSED) {
						ext.value |= (1 << (i + 2));
					} else {
						ext.value &= ~(1 << (i + 2));
					}
					avr_ioctl(avr, AVR_IOCTL_IOPORT_SET_EXTERNAL('D'), &ext);
					avr_raise_irq(irq + i, s);

				}
			}
		}
	}
}

void button_init(){
	// Buttons
	avr_init_irq(&avr->irq_pool, irq, 0, BUTTONS, irq_names);
	avr_raise_irq(irq + BUTTON0, BUTTON_RELEASED);
	avr_raise_irq(irq + BUTTON1, BUTTON_RELEASED);
	avr_connect_irq(irq + BUTTON0, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
	avr_connect_irq(irq + BUTTON1, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
	avr_irq_set_flags(irq + BUTTON0, IRQ_FLAG_NOT);
	avr_irq_set_flags(irq + BUTTON1, IRQ_FLAG_NOT);
//	avr_ioctl(avr, AVR_IOCTL_IOPORT_SET_EXTERNAL('D'), &ext);
	btnState[BUTTON0] = BUTTON_UNCHANGED;
	btnState[BUTTON1] = BUTTON_UNCHANGED;
	avr_connect_irq(irq + BUTTON_USER, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 7));
	btnState[BUTTON_USER] = BUTTON_UNCHANGED;
}
