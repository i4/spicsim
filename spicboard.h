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

cycles_t spicboard_cycles(void);
int spicboard_state(void);
const char * spicboard_filepath(void);
const char * spicboard_state_string(void);
void spicboard_pause(void);
bool spicboard_is_paused(void);
void spicboard_step(void);
void spicboard_run(void);
void spicboard_reset(void);
bool spicboard_stop(void);
bool spicboard_load(const char * fname);
void spicboard_exit(void);

#endif /* SPICBOARD_H */
