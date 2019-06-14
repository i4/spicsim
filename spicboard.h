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


struct avr_t;
extern struct avr_t * avr;

extern const voltage_t voltage;


extern uint16_t gdb_port;
extern bool gdb_enable;

extern state_t spicboard;

cycles_t spicboard_cycles();
int spicboard_state();
const char * spicboard_filepath();
const char * spicboard_state_string();
void spicboard_pause();
bool spicboard_is_paused();
void spicboard_step();
void spicboard_run();
void spicboard_reset();
bool spicboard_stop();
bool spicboard_load(const char * fname);
void spicboard_exit();

#endif /* SPICBOARD_H */
