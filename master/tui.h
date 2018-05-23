#ifndef TUI_H
#define TUI_H
#include <ncurses.h>

typedef struct
{
	int window_width;
	int window_height;
	int col_count;
	int col_width;
} tui_mngr;

void tui_print_line();
void tui_print_debug();
tui_mngr tui_create_mgr(int col_count);
void tui_print_col(const tui_mngr *mngr, const int col_number, const int line_number, const char *str);
void tui_print_borders(const tui_mngr *mngr);
void tui_end();

#endif
