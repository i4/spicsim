#ifndef SPICBOARD_LED_H
#define SPICBOARD_LED_H

#include <stdbool.h>

#include "spicboard_types.h"
#include "sim_avr.h"

enum LED {
	LED_RED0,
	LED_YELLOW0,
	LED_GREEN0,
	LED_BLUE0,
	LED_RED1,
	LED_YELLOW1,
	LED_GREEN1,
	LED_BLUE1,

	LED_USER,

	LED_7SEG_0_0,
	LED_7SEG_0_1,
	LED_7SEG_0_2,
	LED_7SEG_0_3,
	LED_7SEG_0_4,
	LED_7SEG_0_5,
	LED_7SEG_0_6,

	LED_7SEG_1_0,
	LED_7SEG_1_1,
	LED_7SEG_1_2,
	LED_7SEG_1_3,
	LED_7SEG_1_4,
	LED_7SEG_1_5,
	LED_7SEG_1_6,

	LEDS
};

typedef struct {
	bool active;
	cycles_t total; // total active cycles
	cycles_t since; // active since
} led_t;

void led_init(struct avr_t *);
void led_set(enum LED, bool);
double led_lightness(enum LED);

#endif /* SPICBOARD_LED_H */
