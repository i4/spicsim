#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "tui.h"
#include "spicboard.h"

#define xstr(s) str(s)
#define str(s) #s

tui_type_t tui_type = TUI_TYPE_ANSI;
tui_color_t tui_color = TUI_COLOR_256;
bool ansi_escape = true;

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

enum COLOR {
	RED, GREEN, YELLOW, BLUE
};


const int color256_palette[4][6] = {
	{ 16, 52,  88, 124, 160, 196 },
	{ 16, 22,  28,  34,  40,  46 },
	{ 16, 58, 100, 142, 184, 226 }, 
	{ 16, 17,  18,  19,  20,  21 },
};



static int tui_print_led(led_t * led, enum COLOR color, char * append, enum SYMBOL on, enum SYMBOL off) {
	char ** sym = tui[tui_type];

	switch (tui_color) {
		case TUI_COLOR_NONE:
			return printf("%s%s", led->active ? sym[on] : " ", append);
		case TUI_COLOR_ANSI:
			if (led->active)
				return printf("\e[3%dm%s\e[0m%s", 1 + (int)color, sym[on], append);
			else
				return printf("\e[30m%s\e[0m%s", sym[off], append);
		case TUI_COLOR_256:
			{
				int brightness = (int)(sb_led_lightness(led) * 6.0);
				return printf("\e[38;5;%dm%s\e[0m%s", color256_palette[color][brightness], led->active ? sym[on] : sym[off], append);
			}
		case TUI_COLOR_TRUE:
			{
				int r,g,b,c = (int)round(sb_led_lightness(led) * 255);
				switch (color) {
					case RED:    r = c; g = 0; b = 0; break;
					case GREEN:  r = 0; g = c; b = 0; break;
					case YELLOW: r = c; g = c; b = 0; break;
					case BLUE:   r = 0; g = 0; b = c; break;
				}
				return printf("\e[38;2;%d;%d;%dm%s\e[0m%s", r, g, b, led->active ? sym[TUI_LED_ON] : sym[TUI_LED_OFF], append);
			}
	}
	assert(false);
	return 0;
}

#define IFANSI(EN) (ansi_escape ? "\e[" EN "m" : "")
#define RESET (ansi_escape ? "\e[0m" : "")
#define symLED(COLOR, NUMBER, APPEND) tui_print_led(&sb.led[LED_ ## COLOR ## NUMBER], COLOR, APPEND, TUI_LED_ON, TUI_LED_OFF)
#define sym7SEG(SEGMENT, NUMBER, APPEND) tui_print_led(&sb.seg[SEGMENT][NUMBER], RED, APPEND, TUI_7SEG_ ## NUMBER, TUI_7SEG_ ## NUMBER)
#define symBTN(NUMBER) (sb.btn[NUMBER] == 0 ? sym[TUI_BUTTON_PRESSED] : sym[TUI_BUTTON_RELEASED])
void tui_print(){
	char ** sym = tui[tui_type];

	if (ansi_escape){
		puts("\e[11F");
	} else {
		tui_color = TUI_COLOR_NONE;
		puts("\n\n");
	}

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

	print_cycle = current_cycle;
}


