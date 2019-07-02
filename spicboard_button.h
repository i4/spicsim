#ifndef SPICBOARD_BUTTON_H
#define SPICBOARD_BUTTON_H

enum BUTTON {
	BUTTON0 = 0,
	BUTTON1,
	BUTTON_USER,
	BUTTONS
};

enum BUTTONSTATE {
	BUTTON_PRESSED = 0,
	BUTTON_RELEASED = 1,
	BUTTON_UNCHANGED
};

void button_init();
void button_set(enum BUTTON, enum BUTTONSTATE);
void button_raise_irq();

#endif /* SPICBOARD_BUTTON_H */
