/*
 ssd1306_virt.c

 Copyright 2011 Michel Pollet <buserror@gmail.com>
 Copyright 2014 Doug Szumski <d.s.szumski@gmail.com>

 This file is part of simavr.

 simavr is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 simavr is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_time.h"

#include "spicboard_ssd1306.h"
#include "avr_twi.h"
#include "avr_ioport.h"

ssd1306_t oled;


/*
 * Write a byte at the current cursor location and then scroll the cursor.
 */
static void ssd1306_write_data() {
	oled.vram[oled.cursor.page][oled.cursor.column] = oled.spi_data;

	// Scroll the cursor
	if (++(oled.cursor.column) >= SSD1306_VIRT_COLUMNS)
	{
		oled.cursor.column = 0;
		if ( oled.addr_mode == SSD1306_ADDR_MODE_HORZ &&
			(++(oled.cursor.page) >= SSD1306_VIRT_PAGES))
		{
			oled.cursor.page = 0;
		}
	}

	ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
}

/*
 * Called on the first command byte sent. For setting single
 * byte commands and initiating multi-byte commands.
 */
void ssd1306_update_command_register() {
	oled.reg_write_sz = 1;
	switch (oled.spi_data)
	{
		case SSD1306_VIRT_CHARGE_PUMP:
		case SSD1306_VIRT_SET_CONTRAST:
			oled.command_register = oled.spi_data;
			//printf ("SSD1306: CONTRAST SET COMMAND: 0x%02x\n", oled.spi_data);
			return;
		case SSD1306_VIRT_DISP_NORMAL:
			ssd1306_set_flag(SSD1306_FLAG_DISPLAY_INVERTED, 0);
			ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
			//printf ("SSD1306: DISPLAY NORMAL\n");
			oled.command_register = 0x00;

			return;
		case SSD1306_VIRT_DISP_INVERTED:
			ssd1306_set_flag(SSD1306_FLAG_DISPLAY_INVERTED,
			                  1);
			ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
			//printf ("SSD1306: DISPLAY INVERTED\n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_DISP_SUSPEND:
			ssd1306_set_flag(SSD1306_FLAG_DISPLAY_ON, 0);
			ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
			//printf ("SSD1306: DISPLAY SUSPENDED\n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_DISP_ON:
			ssd1306_set_flag(SSD1306_FLAG_DISPLAY_ON, 1);
			ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
			//printf ("SSD1306: DISPLAY ON\n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_PAGE_START_ADDR
		                ... SSD1306_VIRT_SET_PAGE_START_ADDR
		                                + SSD1306_VIRT_PAGES - 1:
			oled.cursor.page = oled.spi_data
			                - SSD1306_VIRT_SET_PAGE_START_ADDR;
			//printf ("SSD1306: SET PAGE ADDRESS: 0x%02x\n", oled.spi_data);
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE
		                ... SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE + 0xF:
			oled.spi_data -= SSD1306_VIRT_SET_COLUMN_LOW_NIBBLE;
			if (oled.addr_mode == SSD1306_ADDR_MODE_PAGE) {
				oled.cursor.column = (oled.cursor.column & 0xF0)
			                | (oled.spi_data & 0xF);
			}
			//printf ("SSD1306: SET COLUMN LOW NIBBLE: 0x%02x\n",oled.spi_data);
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE
		                ... SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE + 0xF:
			oled.spi_data -= SSD1306_VIRT_SET_COLUMN_HIGH_NIBBLE;
			if (oled.addr_mode == SSD1306_ADDR_MODE_PAGE) {
				oled.cursor.column = (oled.cursor.column & 0xF)
			                | ((oled.spi_data & 0xF) << 4);
			}
			//printf ("SSD1306: SET COLUMN HIGH NIBBLE: 0x%02x\n", oled.spi_data);
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_SEG_REMAP_0:
			ssd1306_set_flag(SSD1306_FLAG_SEGMENT_REMAP_0,
			                  1);
			//printf ("SSD1306: SET COLUMN ADDRESS 0 TO OLED SEG0 to \n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_SEG_REMAP_127:
			ssd1306_set_flag(SSD1306_FLAG_SEGMENT_REMAP_0,
			                  0);
			//printf ("SSD1306: SET COLUMN ADDRESS 127 TO OLED SEG0 to \n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_COM_SCAN_NORMAL:
			ssd1306_set_flag(SSD1306_FLAG_COM_SCAN_NORMAL,
			                  1);
			//printf ("SSD1306: SET COM OUTPUT SCAN DIRECTION NORMAL \n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_COM_SCAN_INVERTED:
			ssd1306_set_flag(SSD1306_FLAG_COM_SCAN_NORMAL,
			                  0);
			//printf ("SSD1306: SET COM OUTPUT SCAN DIRECTION REMAPPED \n");
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SCROLL_RIGHT:
		case SSD1306_VIRT_SCROLL_LEFT:
			oled.command_register = oled.spi_data;
			oled.reg_write_sz = 6;
			return;
		case SSD1306_VIRT_SCROLL_VR:
		case SSD1306_VIRT_SCROLL_VL:
			oled.command_register = oled.spi_data;
			oled.reg_write_sz = 5;
			return;
		case SSD1306_VIRT_SET_RATIO_OSC:
		case SSD1306_VIRT_MULTIPLEX:
		case SSD1306_VIRT_SET_OFFSET:
		case SSD1306_VIRT_MEM_ADDRESSING:
		case SSD1306_VIRT_SET_LINE:
		case SSD1306_VIRT_SET_PADS:
		case SSD1306_VIRT_SET_CHARGE:
		case SSD1306_VIRT_SET_VCOM:
			oled.command_register = oled.spi_data;
			return;
		case SSD1306_VIRT_VERT_SCROLL_A:
		case SSD1306_VIRT_SET_PAGE_ADDR:
		case SSD1306_VIRT_SET_COL_ADDR:
			oled.reg_write_sz = 2;
			oled.command_register = oled.spi_data;
			return;
		case SSD1306_VIRT_SCROLL_ON:
		case SSD1306_VIRT_SCROLL_OFF:
		case SSD1306_VIRT_RESUME_TO_RAM_CONTENT:
			oled.command_register = 0x00;
			return;
		default:
			printf ("SSD1306: WARNING: unknown/not implemented command %x\n", oled.spi_data);
			// Unknown command
			return;
	}
}

/*
 * Multi-byte command setting
 */
void ssd1306_update_setting() {
	switch (oled.command_register)
	{
		case SSD1306_VIRT_SET_CONTRAST:
			oled.contrast_register = oled.spi_data;
			ssd1306_set_flag(SSD1306_FLAG_DIRTY, 1);
			oled.command_register = 0x00;
			//printf ("SSD1306: CONTRAST SET: 0x%02x\n", oled.contrast_register);
			return;
		case SSD1306_VIRT_SET_PAGE_ADDR:
			switch (--oled.reg_write_sz) {
				case 1:
					oled.cursor.page = oled.spi_data;
					break;
				case 0:
					//TODO handle virtual page end
					oled.command_register = 0x00;
			}
			return;

		case SSD1306_VIRT_SET_COL_ADDR:
			switch (--oled.reg_write_sz) {
				case 1:
					oled.cursor.column = oled.spi_data;
					break;
				case 0:
					//TODO handle virtual col end
					oled.command_register = 0x00;
			}
			return;
		case SSD1306_VIRT_MEM_ADDRESSING:
			if (oled.spi_data > SSD1306_ADDR_MODE_PAGE)
				printf ("SSD1306: error ADDRESSING_MODE invalid value %x\n", oled.spi_data);
			oled.addr_mode = oled.spi_data;
			//printf ("SSD1306: ADDRESSING MODE: 0x%02x\n", oled.addr_mode);
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SET_LINE:
		case SSD1306_VIRT_SET_RATIO_OSC:
		case SSD1306_VIRT_MULTIPLEX:
		case SSD1306_VIRT_SET_OFFSET:
		case SSD1306_VIRT_SET_CHARGE:
		case SSD1306_VIRT_SET_VCOM:
		case SSD1306_VIRT_SET_PADS:
		case SSD1306_VIRT_CHARGE_PUMP:
			oled.command_register = 0x00;
			return;
		case SSD1306_VIRT_SCROLL_RIGHT:
		case SSD1306_VIRT_SCROLL_LEFT:
		case SSD1306_VIRT_SCROLL_VR:
		case SSD1306_VIRT_SCROLL_VL:
		case SSD1306_VIRT_VERT_SCROLL_A:
		    if (! --oled.reg_write_sz)
			oled.command_register = 0x00;
		    return;
		default:
			// Unknown command
			printf("SSD1306: error: unknown update command %x\n",oled.command_register);
			return;
	}
}

/*
 * Determines whether a new command has been sent, or
 * whether we are in the process of setting a multi-
 * byte command.
 */
static void ssd1306_write_command() {
	if (!oled.command_register)
	{
		// Single byte or start of multi-byte command
		ssd1306_update_command_register();
	} else
	{
		// Multi-byte command setting
		ssd1306_update_setting();
	}
}

/*
 * Called when a TWI byte is sent
 */
static void ssd1306_hook (struct avr_irq_t * irq, uint32_t value, void * param) {
static int i =0;
	avr_twi_msg_irq_t v;
	v.u.v = value;

	if (v.u.twi.msg & TWI_COND_STOP) {
		oled.twi_selected = 0;
		oled.twi_index = 0;
	}

	if (v.u.twi.msg & TWI_COND_START) {
		oled.twi_selected = 0;
		oled.twi_index = 0;
		if (((v.u.twi.addr>>1) & SSD1306_I2C_ADDRESS_MASK) == SSD1306_I2C_ADDRESS) {
			oled.twi_selected = v.u.twi.addr;
			avr_raise_irq(oled.irq + TWI_IRQ_INPUT, avr_twi_irq_msg(TWI_COND_ACK, oled.twi_selected, 1));
		}
	}

	if (oled.twi_selected) {
		if (v.u.twi.msg & TWI_COND_WRITE) {
			avr_raise_irq(oled.irq + TWI_IRQ_INPUT, avr_twi_irq_msg(TWI_COND_ACK, oled.twi_selected, 1));

			if (oled.twi_index == 0) { // control byte
				if (v.u.twi.data != 0xae &&(v.u.twi.data & (~(1<<6))) != 0) {
					printf("%s COND_WRITE %x\n", __FUNCTION__, v.u.twi.data);
					printf("%s ALERT: unhandled Co bit\n", __FUNCTION__);
					abort();
				}
				oled.di_pin = v.u.twi.data ? SSD1306_VIRT_DATA : SSD1306_VIRT_INSTRUCTION;
			} else 
			{
				oled.spi_data = v.u.twi.data;

				switch (oled.di_pin)
				{
					case SSD1306_VIRT_DATA:
						ssd1306_write_data();
						break;
					case SSD1306_VIRT_INSTRUCTION:
						ssd1306_write_command();
						break;
					default:
						// Invalid value
						break;
				}
			}
			oled.twi_index++;
		}

		// SSD1306 doesn't support read on serial interfaces
		// just return 0
		if (v.u.twi.msg & TWI_COND_READ) {
			avr_raise_irq(oled.irq + TWI_IRQ_INPUT, avr_twi_irq_msg(TWI_COND_READ, oled.twi_selected, 0));
			oled.twi_index++;
		}
	}
}


/*
static void
ssd1306_reset_hook (struct avr_irq_t * irq, uint32_t value, void * param)
{
	//printf ("SSD1306: RESET\n");
	ssd1306_t * part = (ssd1306_t*) param;
	if (irq->value && !value)
	{
		// Falling edge
		memset (oled.vram, 0, 64 * 8);
		oled.cursor.column = 0;
		oled.cursor.page = 0;
		oled.flags = 0;
		oled.command_register = 0x00;
		oled.contrast_register = 0x7F;
		oled.addr_mode = SSD1306_ADDR_MODE_PAGE;
		ssd1306_set_flag(SSD1306_FLAG_COM_SCAN_NORMAL, 1);
		ssd1306_set_flag(SSD1306_FLAG_SEGMENT_REMAP_0, 1);
	}

}
*/

static const char * irq_names[2] = {
	[TWI_IRQ_INPUT] = "8>sdd1306.out",
	[TWI_IRQ_OUTPUT] = "32<sdd1306.in",
};

void ssd1306_init (struct avr_t *avr) {
	memset(&oled, 0, sizeof(oled));

	/*
	 * Register callbacks on all our IRQs
	 */
	oled.irq = avr_alloc_irq (&avr->irq_pool, 0, 2, irq_names);
	avr_irq_register_notify (oled.irq + TWI_IRQ_OUTPUT, ssd1306_hook, NULL);
	avr_connect_irq(oled.irq + TWI_IRQ_INPUT, avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ('0'), TWI_IRQ_INPUT));
	avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ('0'), TWI_IRQ_OUTPUT), oled.irq + TWI_IRQ_OUTPUT);
}
