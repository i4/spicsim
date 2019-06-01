#ifndef SPICBOARD_ADC_H
#define SPICBOARD_ADC_H

#include <stdint.h>
#include <stdbool.h>

#include "sim_avr.h"

enum ADC {
	POTI,
	PHOTO,
	ADCS
};

extern uint16_t adc_noise[ADCS];
extern uint16_t adc_value[ADCS];

bool adc_init(struct avr_t *);
void adc_set(enum ADC, uint16_t);
void adc_set_rel(enum ADC, int16_t);

#endif /* SPICBOARD_ADC_H */
