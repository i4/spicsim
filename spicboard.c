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
#include "avr_adc.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "sim_vcd_file.h"
#include "avr_timer.h"

#include "spicboard.h"
#include "spicboard_adc.h"
#include "spicboard_button.h"
#include "spicboard_hc595.h"
#include "spicsim.h"

state_t sb;

const unsigned voltage = 5000; // mV

cycles_t current_cycle = 0;
cycles_t print_cycle = 0;

static avr_t * avr = NULL;

avr_vcd_t vcd_file;

static void pin_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
	*(bool*)param = value;
}

static void led_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
	sb_led_update((enum LED)param, value == 0);
}

void sb_led_update(enum LED led, bool active){
	if (sb.led[led].active != active) {
		cycles_t now = current_cycle;
		if (active)
			sb.led[led].since = now;
		else
			sb.led[led].total += now - sb.led[led].since;
		sb.led[led].active = active;
	}
}

double sb_led_lightness(enum LED led){
	static struct {
		cycles_t total; // last total active cycles
		cycles_t print; // last print
		double value; // last value
	} last[LEDS];
	
	// Fetch Variables
	led_t tmp = sb.led[led];
	cycles_t now = current_cycle;
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
	f.frequency = args_info.frequency_arg;
	f.vcc = f.avcc = f.aref = args_info.voltage_arg;

	avr = avr_make_mcu_by_name(args_info.avr_arg);
	if (avr) {
		avr_init(avr);
		avr_load_firmware(avr, &f);

		// POTI & PHOTO
		adc_value[POTI] = args_info.poti_value_arg;
		adc_noise[POTI] = args_info.poti_noise_arg;
		adc_value[PHOTO] = args_info.photo_value_arg;
		adc_noise[PHOTO] = args_info.photo_noise_arg;
		adc_init(avr);

		// Buttons
		button_init(avr);

		// Shift register / 7 segment display
		hc595_init(avr);

		// connect all the pins to our callback
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led_changed_hook, (void*)LED_RED1);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), led_changed_hook, (void*)LED_YELLOW1);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), led_changed_hook, (void*)LED_USER);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 7), pin_changed_hook, &sb.btn[BUTTON_USER]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 2), led_changed_hook, (void*)LED_BLUE1);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 3), led_changed_hook, (void*)LED_GREEN1);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2), pin_changed_hook, &sb.btn[BUTTON1]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), pin_changed_hook, &sb.btn[BUTTON0]);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 4), led_changed_hook, (void*)LED_GREEN0);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5), led_changed_hook, (void*)LED_YELLOW0);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 6), led_changed_hook, (void*)LED_RED0);
		avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 7), led_changed_hook, (void*)LED_BLUE0);


		// even if not setup at startup, activate gdb if crashing
		avr->gdb_port = args_info.gdb_arg;
		if (args_info.gdb_given) {
			fprintf(stderr, "Starting GDB on port %hd...\n", avr->gdb_port);
			//avr->state = cpu_Stopped;
			avr_gdb_init(avr);
		}
		
		if (args_info.vcd_given) {
			fprintf(stderr, "Writing VCD to '%s' %d...\n", args_info.vcd_arg,  args_info.vcd_flush_arg);
			avr_vcd_init(avr, args_info.vcd_arg, &vcd_file, args_info.vcd_flush_arg);
			avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 1), 1, "INT/EXT_INT0");
			avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 2), 1, "INT/EXT_INT1");
			avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 14), 1, "INT/TIMER0_COMPA");
			avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 15), 1, "INT/TIMER0_COMPB");
			avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 16), 1, "INT/TIMER0_OVF");

			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 6), 1, "LED0-RED0/PD6" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5), 1, "LED1-YELLOW0/PD5" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 4), 1, "LED2-GREEN0/PD4" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 7), 1, "LED3-BLUE0/PD7" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), 1, "LED4-RED1/PB0" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), 1, "LED5-YELLOW1/PB1" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 3), 1, "LED6-GREEN1/PC3" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), 2), 1, "LED7-BLUE1/PC2" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2), 1, "BTN0/PD2" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3), 1, "BTN1/EXT/PD3" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 1), 1, "COM-TX/PD1" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 0), 1, "COM-RX/PD0" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, POTI), 12, "ADC0-POTI/PC0" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, PHOTO), 12, "ADC1-PHOTO/PC1" );

			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_SPI_GETIRQ('0'), SPI_IRQ_OUTPUT), 8 , "HC595-MOSI/SPI0" );
			avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2), 1, "HC595-LATCH/PB2" );

			if (args_info.vcd_extra_given) {
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), 1, "LED-USER/PB5" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 7), 1, "BTN-USER/PB7" );

				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 24), 1, "INT/TWI0" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 3), 1, "INT/PCINT0" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 4), 1, "INT/PCINT1" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 5), 1, "INT/PCINT2" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 27), 1, "INT/PCINT3" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 6), 1, "INT/WDT" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 7), 1, "INT/TIMER2_COMPA");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 8), 1, "INT/TIMER2_COMPB");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 9), 1, "INT/TIMER2_OVF");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 10), 1, "INT/TIMER1_CAPT");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 11), 1, "INT/TIMER1_COMPA");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 12), 1, "INT/TIMER1_COMPB");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 13), 1, "INT/TIMER1_OVF");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 17), 1, "INT/SPI0" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 18), 1, "INT/USART0_RX" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 19), 1, "INT/USART0_UDRE" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 20), 1, "INT/USART0_TX" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 21), 1, "INT/ADC" );
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 32), 1, "INT/TIMER3_CAPT");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 33), 1, "INT/TIMER3_COMPA");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 34), 1, "INT/TIMER3_COMPB");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 35), 1, "INT/TIMER3_OVF");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 41), 1, "INT/TIMER4_CAPT");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 42), 1, "INT/TIMER4_COMPA");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 43), 1, "INT/TIMER4_COMPB");
				avr_vcd_add_signal(&vcd_file, avr_get_interrupt_irq(avr, 44), 1, "INT/TIMER4_OVF");

				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN_ALL), 8, "EXTRA/PORTB" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), IOPORT_IRQ_PIN_ALL), 8, "EXTRA/PORTC" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), IOPORT_IRQ_PIN_ALL), 8, "EXTRA/PORTD" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), IOPORT_IRQ_PIN_ALL), 8, "EXTRA/PORTE" );

				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_DIRECTION_ALL), 8, "EXTRA/DDRB" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('C'), IOPORT_IRQ_DIRECTION_ALL), 8, "EXTRA/DDRC" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), IOPORT_IRQ_DIRECTION_ALL), 8, "EXTRA/DDRD" );
				avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), IOPORT_IRQ_DIRECTION_ALL), 8, "EXTRA/DDRE" );
			}
			avr_vcd_start(&vcd_file);
		}
		// the AVR run on it's own thread. it even allows for debugging!
		pthread_t run;
		return pthread_create(&run, NULL, avr_run_thread, NULL) == 0;
	} else {
		fprintf(stderr, "AVR '%s' not known\n", f.mmcu);
		return false;
	}
}
