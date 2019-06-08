#include "spicboard_vcd.h"
#include "spicboard.h"
#include "spicboard_adc.h"
#include "spicsim.h"

#include "sim_avr.h"
#include "sim_vcd_file.h"
#include "avr_ioport.h"
#include "avr_spi.h"
#include "avr_adc.h"

static avr_vcd_t vcd_file;

bool vcd_init(){
	if (avr_vcd_init(avr, args_info.vcd_file_arg, &vcd_file, args_info.vcd_flush_arg) == 0){
		fprintf(stderr, "Writing VCD to '%s' %d...\n", args_info.vcd_file_arg,  args_info.vcd_flush_arg);
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
		return true;
	} else {
		return false;
	}
}

bool vcd_start(){
	return avr_vcd_start(&vcd_file) == 0;
}

void vcd_stop(){
	avr_vcd_stop(&vcd_file);
}

void vcd_close(){
	avr_vcd_close(&vcd_file);
}
