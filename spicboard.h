#ifndef SPICBOARD_H
#define  SPICBOARD_H

#include <stdbool.h>
#include <time.h>

#include "spicboard_adc.h"
#include "spicboard_button.h"
#include "spicboard_led.h"

typedef struct {
	led_t led[LEDS];
	uint16_t adc[ADCS];
	uint16_t adcread[ADCS];
	bool btn[BUTTONS];
} state_t;

extern const voltage_t voltage;

extern state_t sb;

extern uint16_t gdb_port;
extern bool gdb_enable;

cycles_t get_cycles();
int get_cpu_state();
const char * get_cpu_state_string();

bool spicboard_load(char * fname);

#endif /* SPICBOARD_H */
