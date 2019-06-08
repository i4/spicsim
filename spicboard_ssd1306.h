/*
 ssd1306_virt.h

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

/*
 * This "Part" simulates the SSD1306 OLED display driver.
 *
 * The following functions are currently supported:
 *
 * > Display reset
 * > Display on / suspend
 * > Setting of the contrast
 * > Inversion of the display
 * > Rotation of the display
 * > Writing to the VRAM using horizontal addressing mode
 *
 * It has been tested on a "JY MCU v1.5 OLED" in 4 wire SPI mode
 * with the E/RD and R/W lines hard wired low as per the datasheet.
 *
 */

#ifndef SPICBOARD_SSD1306_H
#define SPICBOARD_SSD1306_H

#define SSD1306_VIRT_PAGES			8
#define SSD1306_VIRT_COLUMNS			128

static const int ssd1306_pages = 8;
static const int ssd1306_rows = 64;
static const int ssd1306_cols = 128;


enum
{
	SSD1306_FLAG_DISPLAY_INVERTED = 0,
	SSD1306_FLAG_DISPLAY_ON,
	SSD1306_FLAG_SEGMENT_REMAP_0,
	SSD1306_FLAG_COM_SCAN_NORMAL,

	/*
	 * Internal flags, not SSD1306
	 */
	SSD1306_FLAG_BUSY,		// 1: Busy between instruction, 0: ready
	SSD1306_FLAG_REENTRANT,		// 1: Do not update pins
	SSD1306_FLAG_DIRTY,			// 1: Needs redisplay...
};

enum ssd1306_addressing_mode_t
{
	SSD1306_ADDR_MODE_HORZ = 0,
	SSD1306_ADDR_MODE_VERT,
	SSD1306_ADDR_MODE_PAGE
};

/*
 * Cursor position in VRAM
 */
struct ssd1306_virt_cursor_t
{
	uint8_t page;
	uint8_t column;
};

struct avr_irq_t;
typedef struct ssd1306_t
{
	struct avr_irq_t * irq;
	struct ssd1306_virt_cursor_t cursor;
	uint8_t vram[SSD1306_VIRT_PAGES][SSD1306_VIRT_COLUMNS];
	uint16_t flags;
	uint8_t command_register;
	uint8_t contrast_register;
	uint8_t cs_pin;
	uint8_t di_pin;
	uint8_t spi_data;
	uint8_t reg_write_sz;
	enum ssd1306_addressing_mode_t addr_mode;

	uint8_t twi_selected;
	uint8_t twi_index;
} ssd1306_t;



extern ssd1306_t oled;

static inline int ssd1306_set_flag(uint16_t bit, int val) {
	int old = oled.flags & (1 << bit);
	oled.flags = (oled.flags & ~(1 << bit)) | (val ? (1 << bit) : 0);
	return old != 0;
}

static inline int ssd1306_get_flag(uint16_t bit) {
	return (oled.flags & (1 << bit)) != 0;
}

void ssd1306_init ();


#endif
