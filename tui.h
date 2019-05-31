#ifndef TUI_H
#define TUI_H

#include <stdbool.h>

typedef enum {
	TUI_TYPE_ANSI,
	TUI_TYPE_UNICODE,
	TUI_TYPES
} tui_type_t;

typedef enum {
	TUI_COLOR_NONE,
	TUI_COLOR_ANSI,
	TUI_COLOR_256,
	TUI_COLOR_TRUE,
} tui_color_t;

extern tui_type_t tui_type;
extern tui_color_t tui_color;
extern bool escape_codes; // false implies TUI_COLOR_NONE

void tui_print();

#endif /* TUI_H */
