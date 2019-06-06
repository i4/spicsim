#include <stddef.h>
#include <stdio.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pthread.h>
#include <assert.h>

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
#include "spicboard_led.h"
#include "spicboard_hc595.h"
#include "spicboard_vcd.h"
#include "spicsim.h"

state_t sb;

const char * cpu_state_str[] = { "Initialization", "Stopped", "Running", "Sleeping", "Step instruction", "Stepped instruction", "Finished", "Crashed" };

const voltage_t voltage = 5000; // mV

static int cpu_state;
static cycles_t current_cycle = 0;

static avr_t * avr = NULL;

static void * avr_run_thread(void * param){
	int s;
	do {
		button_raise_irq();
		s = avr_run(avr);
		__atomic_store_n(&cpu_state, s, __ATOMIC_RELAXED);
		__atomic_fetch_add(&current_cycle, 1, __ATOMIC_RELAXED);
	} while (s != cpu_Done && s != cpu_Crashed);
	return NULL;
}

cycles_t get_cycles() {
	return __atomic_load_n(&current_cycle, __ATOMIC_RELAXED);
}

int get_cpu_state(){
	return __atomic_load_n(&cpu_state, __ATOMIC_RELAXED);
}

const char * get_cpu_state_string(){
	return cpu_state_str[__atomic_load_n(&cpu_state, __ATOMIC_RELAXED)];
}

bool spicboard_init(){
	avr = avr_make_mcu_by_name(args_info.avr_arg);
//	if (avr) {
		avr_init(avr);
		return true;
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
		led_init(avr);

		// even if not setup at startup, activate gdb if crashing
		avr->gdb_port = args_info.gdb_arg;
		if (args_info.gdb_given) {
			fprintf(stderr, "Starting GDB on port %hd...\n", avr->gdb_port);
			//avr->state = cpu_Stopped;
			avr_gdb_init(avr);
		}

		if (args_info.vcd_given) {
			if (!vcd_init(avr))
				fprintf(stderr, "Not able to initialize Value Change Dump recording...\n");
			else if (!vcd_start())
				fprintf(stderr, "Not able to start Value Change Dump recording...\n");
		}
		// the AVR run on it's own thread. it even allows for debugging!
		pthread_t run;
		return pthread_create(&run, NULL, avr_run_thread, NULL) == 0;
	} else {
		fprintf(stderr, "AVR '%s' not known\n", f.mmcu);
		return false;
	}
}

void spicboard_exit() {
	avr_deinit_gdb(avr);
	avr_terminate(avr);
	vcd_stop();

}
