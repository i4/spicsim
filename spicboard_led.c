#include <math.h>
#include <assert.h>

#include "spicboard_led.h"
#include "spicboard.h"

#include "sim_avr.h"
#include "avr_ioport.h"


static void led_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
	led_set((enum LED)param, value == 0);
}

void led_set(enum LED led, bool active){
	if (sb.led[led].active != active) {
		cycles_t now = get_cycles();
		if (active)
			sb.led[led].since = now;
		else
			sb.led[led].total += now - sb.led[led].since;
		sb.led[led].active = active;
	}
}

double led_lightness(enum LED led){
	static struct {
		cycles_t total; // last total active cycles
		cycles_t print; // last print
		double value; // last value
	} last[LEDS];
	
	// Fetch Variables
	led_t tmp = sb.led[led];
	cycles_t now = get_cycles();
	if (now == last[led].print)
		return last[led].value;
	assert(now > last[led].print);
	
	double L = 0.99999999;
	if (!tmp.active || tmp.since >= last[led].print) {
		// calculate
		// https://en.wikipedia.org/wiki/Lightness#Relationship_to_value_and_relative_luminance
		// CIE 1931 - http://www.photonstophotos.net/GeneralTopics/Exposure/Psychometric_Lightness_and_Gamma.htm
		cycles_t total = ((tmp.total - last[led].total) + (tmp.active ? now - tmp.since : 0));
		double Y = 1.0 * total / (now - last[led].print);
		if (Y < 1.0) {
			// Y ^ (1/3) = cubicroot(Y)
			L = Y <= 0.008856 ? (9.033 * Y) : (1.16 * cbrt(Y) - 0.16000001);
		}
	}
	assert( L >= 0.0 && L < 1.0);

	// update local values
	last[led].total = tmp.total;
	last[led].print = now;
	last[led].value = L;
	return L;
}

void led_init(){
	// connect all the pins to our callback
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led_changed_hook, (void*)LED_RED1);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), led_changed_hook, (void*)LED_YELLOW1);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), led_changed_hook, (void*)LED_USER);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 2), led_changed_hook, (void*)LED_BLUE1);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 3), led_changed_hook, (void*)LED_GREEN1);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 4), led_changed_hook, (void*)LED_GREEN0);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5), led_changed_hook, (void*)LED_YELLOW0);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 6), led_changed_hook, (void*)LED_RED0);
	avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 7), led_changed_hook, (void*)LED_BLUE0);
}

