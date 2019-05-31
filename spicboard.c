#include <stddef.h>
#include <stdio.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_spi.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "avr_timer.h"

#include "spicboard.h"
#include "spicboard_adc.h"
#include "spicboard_button.h"
#include "spicboard_hc595.h"

state_t sb;
uint16_t gdb_port = 1234;
bool gdb_enable = false;
cycles_t current_cycle = 0;
cycles_t print_cycle = 0;

static avr_t * avr = NULL;
// TODO: static state_t sb_old;

static void pin_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
	*(bool*)param = value;
}

static void led_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
	sb_led_update((led_t*)param, value == 0);
}

void sb_led_update(led_t * led, bool active){
	if (led->active != active) {
		led_t tmp = *led;
		cycles_t now = current_cycle;
		if (tmp.active)
			tmp.total += now - tmp.last;
		tmp.last = now;
		tmp.active = active;
		*led = tmp; 
	}
}

double sb_led_lightness(led_t * led){
	// https://en.wikipedia.org/wiki/Lightness#Relationship_to_value_and_relative_luminance
	// CIE 1931 - http://www.photonstophotos.net/GeneralTopics/Exposure/Psychometric_Lightness_and_Gamma.htm
	led_t tmp = *led;
	cycles_t now = current_cycle;

	// reset values
	led->last = led->print = now;
	led->total = 0;

	cycles_t cycles = (now - tmp.print);
	double Y = cycles == 0 ? 0 : (1.0 * (tmp.total + (tmp.active ? now - tmp.last : 0)) / cycles);
	// Y ^ (1/3) = cubicroot(Y)
	double L = Y <= 0.008856 ? (9.033 * Y) : (1.16 * cbrt(Y) - 0.16000001);
	assert( L >= 0.0 && L < 1.0);
	return L;
}


static void * avr_run_thread(void * oaram){

	while (1) {
		avr_run(avr);
		current_cycle++;
	}
	return NULL;
}

bool spicboard_load(char * fname){
	elf_firmware_t f;
	elf_read_firmware(fname, &f);
	f.frequency = 16000000;

	avr = avr_make_mcu_by_name("atmega328pb");
	if (avr) {
		avr_init(avr);
		avr_load_firmware(avr, &f);

		// POTI & PHOTO
		adc_init(avr);

		// Buttons
		button_init(avr);

		// Shift register / 7 segment display
		hc595_init(avr);

		// connect all the pins to our callback
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led_changed_hook, &sb.led[LED_RED1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), led_changed_hook, &sb.led[LED_YELLOW1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 2), led_changed_hook, &sb.led[LED_BLUE1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 3), led_changed_hook, &sb.led[LED_GREEN1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2), pin_changed_hook, &sb.btn[BUTTON1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), pin_changed_hook, &sb.btn[BUTTON0]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 4), led_changed_hook, &sb.led[LED_GREEN0]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5), led_changed_hook, &sb.led[LED_YELLOW0]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 6), led_changed_hook, &sb.led[LED_RED0]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 7), led_changed_hook, &sb.led[LED_BLUE0]);


		// even if not setup at startup, activate gdb if crashing
		avr->gdb_port = gdb_port;
		if (gdb_enable) {
			//avr->state = cpu_Stopped;
			avr_gdb_init(avr);
		}
		
		// the AVR run on it's own thread. it even allows for debugging!
		pthread_t run;
		return pthread_create(&run, NULL, avr_run_thread, NULL) == 0;
	} else {
		fprintf(stderr, "AVR '%s' not known\n", f.mmcu);
		return false;
	}
}
