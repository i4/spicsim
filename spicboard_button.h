#ifndef SPICBOARD_BUTTON_H
#define SPICBOARD_BUTTON_H

#include "sim_avr.h"

enum BUTTON {
	BUTTON0,
	BUTTON1,
	BUTTONS
};

enum BUTTONSTATE {
	BUTTON_PRESSED,
	BUTTON_RELEASED
};

void button_init(struct avr_t *);
void button_set(enum BUTTON, enum BUTTONSTATE);

#endif /* SPICBOARD_BUTTON_H */
