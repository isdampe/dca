#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "tui.h"

tui_mngr tui_create_mgr(int col_count)
{
	tui_mngr mngr;
	mngr.col_count = col_count;

	//Start curses mode.
	initscr();
	noecho();
	timeout(10);
	cbreak();
	keypad(stdscr, TRUE);

	mngr.window_width = COLS;
	mngr.window_height = LINES;
	mngr.col_width = (COLS / col_count) - col_count;

	return mngr;
}

void tui_print_col(const tui_mngr *mngr, const int col_number, const int line_number, const char *str)
{
	if (col_number > mngr->col_count)
		return;

	 char *line_cutoff = calloc(mngr->col_width, sizeof(char));
	 strncpy(line_cutoff, str, mngr->col_width -3);

	 mvprintw(line_number, (col_number -1) * mngr->col_width, line_cutoff);
	 refresh();
}

void tui_print_borders(const tui_mngr *mngr)
{
	for (int i=0; i<mngr->col_count; ++i)
		for (int x=0; x<mngr->window_height; ++x)
			mvprintw(x, (i * mngr->col_width) + mngr->col_width -2, "|");
}

