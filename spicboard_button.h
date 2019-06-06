#ifndef SPICBOARD_BUTTON_H
#define SPICBOARD_BUTTON_H

#include "sim_avr.h"

enum BUTTON {
	BUTTON0,
	BUTTON1,
	BUTTON_USER,
	BUTTONS
};

enum BUTTONSTATE {
	BUTTON_PRESSED,
	BUTTON_RELEASED,
	BUTTON_UNCHANGED
};

void button_init(struct avr_t *);
void button_set(enum BUTTON, enum BUTTONSTATE);
void button_raise_irq();

#endif /* SPICBOARD_BUTTON_H */
