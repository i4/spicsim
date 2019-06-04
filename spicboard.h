#ifndef SPICBOARD_H
#define  SPICBOARD_H

#include <stdbool.h>
#include <time.h>

#include "spicboard_adc.h"
#include "spicboard_button.h"

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

typedef uint64_t cycles_t;

typedef struct {
	bool active;
	cycles_t total; // total active cycles
	cycles_t since; // active since
} led_t;

typedef struct {
	led_t led[LEDS];
	uint16_t adc[ADCS];
	bool btn[BUTTONS];
} state_t;

extern const unsigned voltage;

void sb_led_update(enum LED, bool);
double sb_led_lightness(enum LED);

extern state_t sb;

extern uint16_t gdb_port;
extern bool gdb_enable;

extern cycles_t current_cycle, print_cycle;

bool spicboard_load(char * fname);

#endif /* SPICBOARD_H */
