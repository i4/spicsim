#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "sim_avr.h"
#include "avr_ioport.h"
#include "avr_spi.h"
#include "sim_elf.h"
#include "sim_gdb.h"
#include "avr_timer.h"

#include "spicboard.h"
#include "spicboard_adc.h"
#include "spicboard_button.h"
#include "tui.h"

int main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("No file\n");
		return EXIT_FAILURE;
	}
	spicboard_load(argv[1]);


	printf("\n\n\n\n\n\n\n\n\n\n\n\n");
	tui_print();

	static struct termios oldt, newt;

	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;

	newt.c_lflag &= ~(ICANON);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	const int KEYBUF = 10;
	while(1){

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);

		struct timeval timeout = { .tv_sec = 0, .tv_usec = 100000 } ;

		errno = 0;
		int rv = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
		if(rv == -1 )
			perror("select"); /* an error accured */
		else if (rv != 0){
			char keys[KEYBUF];
			rv = read(STDIN_FILENO, keys, KEYBUF);
			if (rv == -1)
				perror("read");
			if (keys[0]=='x')
				button_set(BUTTON0, BUTTON_PRESSED);
			if (keys[0]=='X')
				button_set(BUTTON0, BUTTON_RELEASED);

			if (keys[0]=='y')
				button_set(BUTTON1, BUTTON_PRESSED);
			if (keys[0]=='Y')
				button_set(BUTTON1, BUTTON_RELEASED);


			if (keys[0]=='a')
				adc_set_rel(POTI,1);
			else if (keys[0]=='s')
				adc_set_rel(POTI,-1);

			if (keys[0]=='A')
				adc_set_rel(POTI,10);
			else if (keys[0]=='S')
				adc_set_rel(POTI,-10);


		}

		tui_print();
	}
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}
