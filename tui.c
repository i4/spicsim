#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "tui.h"
#include "spicsim.h"
#include "spicboard.h"
#include "spicboard_led.h"
#include "spicboard_ssd1306.h"

#define xstr(s) str(s)
#define str(s) #s

enum SYMBOL {
	TUI_TOP_LEFT,
	TUI_HORIZONTAL,
	TUI_TOP_RIGHT,
	TUI_VERTICAL,
	TUI_BOTTOM_LEFT,
	TUI_BOTTOM_RIGHT,
	TUI_LED_OFF,
	TUI_LED_ON,
	TUI_PHOTO,
	TUI_POTI,
	TUI_BUTTON_RELEASED,
	TUI_BUTTON_PRESSED,
	TUI_7SEG_0,
	TUI_7SEG_1,
	TUI_7SEG_2,
	TUI_7SEG_3,
	TUI_7SEG_4,
	TUI_7SEG_5,
	TUI_7SEG_6,

	TUI_SYMBOLS
};

static char * tui[2][TUI_SYMBOLS]= {
	{"+","----------------------","+","|","+","+","O","O","o","o","[ ]","[X]","_","|","|","_","|","|","_"},
	{"┌","──────────────────────","┐","│","└","┘","○","●","◌","◉"," □ "," ■ ","▁","▏","▏","▁","▕","▕","▁" }
};

// Color
enum COLOR {
	RED, GREEN, YELLOW, BLUE
};

const int color256_palette[4][6] = {
	{ 16, 52,  88, 124, 160, 196 },
	{ 16, 22,  28,  34,  40,  46 },
	{ 16, 58, 100, 142, 184, 226 }, 
	{ 16, 17,  18,  19,  20,  21 },
};

// Display 
static char * ublock[] = { " ", "▘","▖","▌", "▝","▀","▞","▛", "▗","▚","▄","▙", "▐","▜","▟","█" };

static const int pages = 8;
static const int rows = 64;
static const int cols = 128;

static void tui_display(){
	ssd1306_set_flag(SSD1306_FLAG_DIRTY, 0);
	const uint8_t vertical = ssd1306_get_flag(SSD1306_FLAG_SEGMENT_REMAP_0);
	const uint8_t horizontal = ssd1306_get_flag(SSD1306_FLAG_COM_SCAN_NORMAL);
	bool invert = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_INVERTED) == 0;
	bool active = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_ON);
	
	uint8_t buf[rows][cols];
	for (int p = 0; p < pages; p++){
		for (int b = 0; b < 8; b++){
			for (int c = 0; c < cols; c++){
				uint8_t r = p * 8 + b;
				buf[horizontal ? (rows - r - 1) : r][vertical ? (cols - c - 1) : c] = active ? (((oled.vram[p][c] >> b) & 1) == invert) : 0;
			}
		}
	}

	if (!args_info.noesc_given)
		switch (args_info.color_arg) {
			case color_arg_none: break;
			case color_arg_16: printf("\e[37m"); break;
			default: printf("\e[38;5;%dm",  (oled.contrast_register) / 17 + 240);
		}
	
	if (args_info.terminal_arg == terminal_arg_utf8){
		fputs("┌", stdout);
		for (int c = 0; c < cols; c+=2)
			fputs("─", stdout);
		puts("┐");
		for (int r = 0; r < rows; r+=2){
			fputs("│", stdout);
			for (int c = 0; c < cols; c+=2)
				fputs(ublock[buf[r][c] | (buf[r + 1][c] << 1) | (buf[r][c + 1] << 2) | (buf[r + 1][c + 1] << 3)], stdout);
			puts("│");
		}
		fputs("└", stdout);
		for (int c = 0; c < cols; c+=2)
			fputs("─", stdout);
		puts("┘");
	} else {
		char line[cols + 3];
		line[cols + 2]='\0';

		line[0]='+';
		line[cols + 1]='+';
		for (int c = 1; c <= cols; c++)
			line[c] = '-';
		puts(line);
				
		line[0]='|';
		line[cols + 1]='|';

		for (int p = 0; p < rows; p++){
			for (int c = 0; c < cols; c++)
				line[c+1] = buf[p][c] == invert ? ' ' : 'X';
			puts(line);
		} 
		
		line[0]='+';
		line[cols + 1]='+';
		for (int c = 1; c <= cols; c++)
			line[c] = '-';
		puts(line);
	}
}


static int tui_print_led(enum LED led, enum COLOR color, char * append, enum SYMBOL on, enum SYMBOL off) {
	char ** sym = tui[args_info.terminal_arg == terminal_arg_utf8 ? 1 : 0];

	switch (args_info.color_arg) {
		case color_arg_none:
			return printf("%s%s", sb.led[led].active ? sym[on] : " ", append);
		case color_arg_256:
			{
				double lightness = led_lightness(led);
				int brightness = (int)(lightness * 6.0);
				return printf("\e[38;5;%dm%s\e[0m%s", color256_palette[color][brightness], lightness >= args_info.lightness_arg ? sym[on] : sym[off], append);
			}
		case color_arg_truecolor:
			{
				double lightness = led_lightness(led);
				int r,g,b,c = (int)(lightness * 255.0);
				switch (color) {
					case RED:    r = c; g = 0; b = 0; break;
					case GREEN:  r = 0; g = c; b = 0; break;
					case YELLOW: r = c; g = c; b = 0; break;
					case BLUE:   r = 0; g = 0; b = c; break;
				}
				return printf("\e[38;2;%d;%d;%dm%s\e[0m%s", r, g, b, lightness >= args_info.lightness_arg ? sym[TUI_LED_ON] : sym[TUI_LED_OFF], append);
			}
		default: // color_arg_16:
			if (led_lightness(led) > args_info.lightness_arg)
				return printf("\e[3%dm%s\e[0m%s", 1 + (int)color, sym[on], append);
			else
				return printf("\e[30m%s\e[0m%s", sym[off], append);

	}
	assert(false);
	return 0;
}

#define IFANSI(EN) (args_info.noesc_given ? "" : "\e[" EN "m")
#define RESET IFANSI("0")
#define symLED(COLOR, NUMBER, APPEND) tui_print_led(LED_ ## COLOR ## NUMBER, COLOR, APPEND, TUI_LED_ON, TUI_LED_OFF)
#define sym7SEG(SEGMENT, NUMBER, APPEND) tui_print_led(LED_7SEG_ ## SEGMENT ## _ ## NUMBER, RED, APPEND, TUI_7SEG_ ## NUMBER, TUI_7SEG_ ## NUMBER)
#define symBTN(NUMBER) (sb.btn[NUMBER] == 0 ? sym[TUI_BUTTON_PRESSED] : sym[TUI_BUTTON_RELEASED])
void tui_print(){
	char ** sym = tui[args_info.terminal_arg == terminal_arg_utf8 ? 1 : 0];

	if (args_info.noesc_given){
		args_info.color_arg = color_arg_none;
		puts("\n\n");
	} else {
		unsigned l = 10;
		if (args_info.display_given)
			l += args_info.terminal_arg == terminal_arg_utf8 ? 34 : 66;
		printf("\e[%uF", l);
	}

	if (args_info.display_given)
		tui_display();

	printf("\t%s%s%s\n\t%s ", sym[TUI_TOP_LEFT], sym[TUI_HORIZONTAL], sym[TUI_TOP_RIGHT], sym[TUI_VERTICAL]);
	symLED(RED,0,"   ");
	printf("%sSPiCboard v3%s     %s\n\t%s ", IFANSI("1;3"), RESET, sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(YELLOW,0,"   ");
	printf("%sSimulator%s        %s\n\t%s ", IFANSI("1;3"), RESET, sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(GREEN,0,"             ");
	printf("%s%4dmV%s %s\n\t%s ", IFANSI("1;37"), sb.adc[PHOTO], RESET, sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(BLUE,0,"                ");
	printf("%s   %s\n\t%s ", sym[TUI_PHOTO], sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(RED,1,"         ");
	sym7SEG(0,0,"   ");
	sym7SEG(1,0,"      ");
	printf("%s\n\t%s ", sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(YELLOW,1,"        ");
	sym7SEG(0,5,"");
	sym7SEG(0,6,"");
	sym7SEG(0,1," ");
	sym7SEG(1,5,"");
	sym7SEG(1,6,"");
	sym7SEG(1,1,"     ");
	printf("%s\n\t%s ", sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(GREEN,1,"   ");
	printf("%s    ", sym[TUI_POTI]);
	sym7SEG(0,4,"");
	sym7SEG(0,3,"");
	sym7SEG(0,2," ");
	sym7SEG(1,4,"");
	sym7SEG(1,3,"");
	sym7SEG(1,2," ");
	printf("%s %s\n\t%s ", symBTN(0), sym[TUI_VERTICAL], sym[TUI_VERTICAL]);
	symLED(BLUE,1," ");
	printf("%s%4dmV%s         %s %s\n\t%s%s%s\n", IFANSI("1;37"), sb.adc[POTI], RESET, symBTN(1), sym[TUI_VERTICAL],sym[TUI_BOTTOM_LEFT], sym[TUI_HORIZONTAL], sym[TUI_BOTTOM_RIGHT]);
}

