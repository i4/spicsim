#ifndef SPICBOARD_H
#define  SPICBOARD_H

#include <stdbool.h>
#include <time.h>

#include "spicboard_adc.h"
#include "spicboard_button.h"

enum {
	LED_RED0,
	LED_YELLOW0,
	LED_GREEN0,
	LED_BLUE0,
	LED_RED1,
	LED_YELLOW1,
	LED_GREEN1,
	LED_BLUE1,
	LEDS
};

typedef uint64_t cycles_t;

typedef struct {
	cycles_t last;
	cycles_t total;
	cycles_t print;
	bool active;
} led_t;

typedef struct {
	led_t led[LEDS];
	led_t seg[2][7];
	uint16_t adc[ADCS];
	bool btn[BUTTONS];
} state_t;

extern const unsigned voltage;

void sb_led_update(led_t *, bool);
double sb_led_lightness(led_t *);

extern state_t sb;

extern uint16_t gdb_port;
extern bool gdb_enable;

extern cycles_t current_cycle, print_cycle;

bool spicboard_load(char * fname);

#endif /* SPICBOARD_H */
