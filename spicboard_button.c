#include <stdint.h>
#include <stdbool.h>

#include "spicboard_button.h"
#include "spicboard.h"

#include "sim_avr.h"
#include "avr_ioport.h"

static avr_irq_t * irq;

static const char * irq_names[BUTTONS] = {
	[BUTTON0] = "BTN0",
	[BUTTON1] = "BTN1",
	[BUTTON_USER] = "BTNUSER",
};

static bool update = false;
static enum BUTTONSTATE btnState[BUTTONS];

void button_set(enum BUTTON btn, enum BUTTONSTATE state){
	if (state != BUTTON_UNCHANGED) {
		sb.btn[btn] = state;
		__atomic_store_n(btnState + btn, state, __ATOMIC_RELAXED);
		__atomic_store_n(&update, true, __ATOMIC_RELAXED);
	}
}

void button_raise_irq() {
	if (__atomic_load_n(&update, __ATOMIC_RELAXED)) {
		__atomic_store_n(&update, false, __ATOMIC_RELAXED);
		for (int i = 0 ; i < BUTTONS; i++) {
			enum BUTTONSTATE s = __atomic_exchange_n(btnState + i, BUTTON_UNCHANGED, __ATOMIC_RELAXED);
			if (s != BUTTON_UNCHANGED)
				avr_raise_irq(irq + i, s);
		}
	}
}

void button_init(){
	// Buttons
	irq = avr_alloc_irq(&avr->irq_pool, 0, BUTTONS, irq_names);
	avr_connect_irq(irq + BUTTON0, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2));
	avr_connect_irq(irq + BUTTON1, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3));
	avr_connect_irq(irq + BUTTON_USER, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 7));
	for (int i = 0 ; i < BUTTONS ; i++) {
		btnState[i] = BUTTON_UNCHANGED;
	}

}
