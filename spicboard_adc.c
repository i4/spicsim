#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "spicboard_adc.h"
#include "spicboard.h"

#include "sim_avr.h"
#include "avr_adc.h"
#include "sim_irq.h"

voltage_t adc_noise[ADCS];
voltage_t adc_value[ADCS];

enum {
	IRQ_TRIGGER = 0,
	IRQ_UPDATE,
	IRQ_COUNT
};

static void adc_trigger(struct avr_irq_t * irq, uint32_t value, void * param) {
	avr_adc_mux_t v = *((avr_adc_mux_t*)&value);
	if (v.src == (enum ADC)param){
		voltage_t val = __atomic_load_n(adc_value + v.src, __ATOMIC_RELAXED);
		if (adc_noise[v.src] != 0) {
			voltage_t noise = (rand() % (adc_noise[v.src] * 2 + 1)) - adc_noise[v.src];
			if (noise < 0 && -noise > val)
				val = 0;
			else
				val += noise;
		}
		if (val > voltage)
			val = voltage;
		else if (val < 0)
			val = 0;
		avr_raise_irq(irq + IRQ_UPDATE, val);
	}
}


static void adc_update(struct avr_irq_t * irq, uint32_t value, void * param) {
	sb.adcread[(enum ADC)param] = value & 0xffff;
}

static const char * irq_names[ADCS][IRQ_COUNT] = {
	{
		[IRQ_TRIGGER] = "poti.trigger",
		[IRQ_UPDATE] = "poti.out",
	},
	{
		[IRQ_TRIGGER] = "photo.trigger",
		[IRQ_UPDATE] = "photo.out",
	}
};

bool adc_init(){
	for (enum ADC adc = 0; adc < ADCS; adc++){
		sb.adc[adc] = adc_value[adc];

		avr_irq_t * irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_COUNT, irq_names[adc]);

		avr_irq_t * src = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_OUT_TRIGGER),
			      * dst = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, adc);

		if (src && dst) {
			avr_connect_irq(src, irq + IRQ_TRIGGER);
			avr_connect_irq(irq + IRQ_UPDATE, dst);

			avr_irq_register_notify(irq + IRQ_TRIGGER, adc_trigger, (void *)adc);
			avr_irq_register_notify(irq + IRQ_UPDATE, adc_update, (void *)adc);
		} else {
			return false;
		}
	}
	return true;
}

void adc_set(enum ADC adc, voltage_t value) {
	__atomic_store_n(adc_value + adc, value, __ATOMIC_RELAXED);
	sb.adc[adc] = value;
}

void adc_set_rel(enum ADC adc, voltage_t value) {
	sb.adc[adc] = __atomic_fetch_add(adc_value + adc, value, __ATOMIC_RELAXED);
}
