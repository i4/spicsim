#include <stddef.h>
#include <stdio.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
#include "spicboard_ssd1306.h"
#include "spicboard_vcd.h"
#include "spicsim.h"

state_t spicboard;

static char current_file[PATH_MAX + 1];
static char mc_status[512];
static pthread_t avr_thread;

const char * cpu_state_str[] = { "initialization", "stopped", "running", "sleeping", "step instruction", "stepped instruction", "finished", "crashed" };

const voltage_t voltage = 5000; // mV

const unsigned sync_us = 1000; // Sync every 1 ms

static int cpu_state;
static cycles_t current_cycle = 0;

static enum {
	AVR_RUN,
	AVR_PAUSE,
	AVR_STEP,
	AVR_RESET,
	AVR_TERMINATE
} avr_action;

struct avr_t * avr = NULL;

static void * avr_run_thread(void * param){
	struct avr_t * avr = param;
	int state;
	unsigned sync_counter = 0;
	const unsigned sync_counter_reset = avr->frequency / sync_us;
	__atomic_store_n(&current_cycle, 0, __ATOMIC_RELAXED);
	do {
		// ensure correct timing
		if (sync_counter++ >= sync_counter_reset){
			avr_callback_sleep_raw(avr, 0);
			sync_counter = 0;
		}
		button_raise_irq();
		state = avr_run(avr);
		__atomic_store_n(&cpu_state, state, __ATOMIC_RELAXED);
		__atomic_fetch_add(&current_cycle, 1, __ATOMIC_RELAXED);
		switch (__atomic_load_n(&avr_action, __ATOMIC_RELAXED)){
			case AVR_STEP:
				__atomic_store_n(&avr_action, AVR_PAUSE, __ATOMIC_RELAXED);
				 __attribute__ ((fallthrough)); 
			case AVR_PAUSE:
				{
					const uint64_t start = avr_get_time_stamp(avr);
					do {
						usleep(10000);
					} while (__atomic_load_n(&avr_action, __ATOMIC_RELAXED) == AVR_PAUSE);
					avr->time_base += avr_get_time_stamp(avr) - start;
				}
				break;
			case AVR_RESET:
				puts("reset");
				led_reset();
				avr_reset(avr);
				__atomic_store_n(&current_cycle, 0, __ATOMIC_RELAXED);
				__atomic_store_n(&avr_action, AVR_RUN, __ATOMIC_RELAXED);
				break;
			case AVR_TERMINATE:
				state = cpu_Done;
				break;
			default:
				break;
		}
	} while (state != cpu_Done && state != cpu_Crashed);
	avr_terminate(avr);
	return NULL;
}

cycles_t spicboard_cycles(void) {
	return __atomic_load_n(&current_cycle, __ATOMIC_RELAXED);
}

int spicboard_state(void){
	return __atomic_load_n(&cpu_state, __ATOMIC_RELAXED);
}

const char * spicboard_filepath(void){
	return current_file;
}

const char * spicboard_state_string(void){
	static char status[1024];
	return snprintf(status, sizeof(status) - 1, "%s: %'llu cycles (%s)", mc_status, (unsigned long long)spicboard_cycles(), cpu_state_str[spicboard_state()]) > 0 ? status : NULL;
}

static void debug_log(struct avr_t *avr, const int level, const char *format, va_list ap){
	fprintf(stderr, "[Log %d] ", level);
	vfprintf(stderr, format, ap);
}

void spicboard_pause(void){
	__atomic_store_n(&avr_action, AVR_PAUSE, __ATOMIC_RELAXED);
}

bool spicboard_is_paused(void){
	return __atomic_load_n(&avr_action, __ATOMIC_RELAXED) == AVR_PAUSE;
}


void spicboard_step(void){
	__atomic_store_n(&avr_action, AVR_STEP, __ATOMIC_RELAXED);
}

void spicboard_run(void){
	__atomic_store_n(&avr_action, AVR_RUN, __ATOMIC_RELAXED);
}

void spicboard_reset(void){
	__atomic_store_n(&avr_action, AVR_RESET, __ATOMIC_RELAXED);
}

bool spicboard_stop(void){
	__atomic_store_n(&avr_action, AVR_TERMINATE, __ATOMIC_RELAXED);
	if (pthread_join(avr_thread, NULL) == 0){
		return true;
	} else {
		return false;
	}
}

bool spicboard_load(const char * fname){
	if (realpath(fname, current_file) == NULL){
		current_file[0]='\0';
		perror("Retrieving real path of file");
		return false;
	} else {
		char name[256];
		int frequency = args_info.frequency_arg;
		char * unit = "Hz";
		if (frequency > 1000){
			frequency /= 1000;
			unit = "KHz";
		}
		if (frequency > 1000){
			frequency /= 1000;
			unit = "MHz";
		}
		if (snprintf(mc_status, sizeof(mc_status) - 1, "%s on %s (%d %s / %.1f V)", basename(strncpy(name, current_file, sizeof(name) - 1)), args_info.avr_arg, frequency, unit, args_info.voltage_arg / 1000.0) < 0)
			mc_status[0] = '\0';
	}

	elf_firmware_t f;
	elf_read_firmware(current_file, &f);
	f.frequency = args_info.frequency_arg;
	f.vcc = f.avcc = f.aref = args_info.voltage_arg;

	avr = avr_make_mcu_by_name(args_info.avr_arg);
	if (avr) {
		avr_init(avr);
		avr_load_firmware(avr, &f);
		if (args_info.debug_given)
			avr_global_logger_set(debug_log);

		// POTI & PHOTO
		adc_value[POTI] = args_info.poti_value_arg;
		adc_noise[POTI] = args_info.poti_noise_arg;
		adc_value[PHOTO] = args_info.photo_value_arg;
		adc_noise[PHOTO] = args_info.photo_noise_arg;
		adc_init();

		// Buttons
		button_init();

		// Shift register / 7 segment display
		hc595_init();

		// connect all the pins to our callback
		led_init();

		ssd1306_init();

		avr->gdb_port = args_info.gdb_arg;
		if (args_info.gdb_given) {
			fprintf(stderr, "Starting GDB on port %hd...\n", avr->gdb_port);
			avr_gdb_init(avr);
		    spicboard_pause();
		} else {
		    spicboard_run();
		}

		if (args_info.vcd_given) {
			if (!vcd_init())
				fprintf(stderr, "Not able to initialize Value Change Dump recording...\n");
			else if (!vcd_start())
				fprintf(stderr, "Not able to start Value Change Dump recording...\n");
		}

		spicboard_run();
		return pthread_create(&avr_thread, NULL, avr_run_thread, avr) == 0;
	} else {
		fprintf(stderr, "AVR '%s' not known\n", f.mmcu);
		return false;
	}
}

void spicboard_exit(void) {
	if (!spicboard_stop()){
		avr_deinit_gdb(avr);
		vcd_stop();
	}
}
