#ifndef SPICBOARD_ADC_H
#define SPICBOARD_ADC_H

#include <stdint.h>
#include <stdbool.h>

#include "spicboard_types.h"

enum ADC {
	POTI,
	PHOTO,
	ADCS
};

extern voltage_t adc_noise[ADCS];
extern voltage_t adc_value[ADCS];

bool adc_init();
void adc_set(enum ADC, voltage_t);
void adc_set_rel(enum ADC, voltage_t);

#endif /* SPICBOARD_ADC_H */
