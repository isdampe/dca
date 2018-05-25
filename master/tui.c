#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "tui.h"

/**
 * Creates a basic terminal user interface struct.
 * Sets up screen mode for ncurses lib.
 * @param  col_count The number of columns in the tui.
 * @return           A new tui_mngr struct.
 */
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

/**
 * Prints a given string to a given line in a given column of a given tui_mngr.
 * @param mngr        A pointer to the tui instance
 * @param col_number  The non-zero indexed column number
 * @param line_number The line number
 * @param str         The string to write to screen
 */
void tui_print_col(const tui_mngr *mngr, const int col_number, const int line_number, const char *str)
{
	if (col_number > mngr->col_count)
		return;

	 char *line_cutoff = calloc(mngr->col_width + 1, sizeof(char));
	 strncpy(line_cutoff, str, mngr->col_width -3);

	 mvprintw(line_number, (col_number -1) * mngr->col_width, line_cutoff);
	 free(line_cutoff);

}

/**
 * Prints the borders on each column
 * @param mngr A pointer to the tui_mngr instance.
 */
void tui_print_borders(const tui_mngr *mngr)
{
	for (int i=0; i<mngr->col_count; ++i)
		for (int x=0; x<mngr->window_height; ++x)
			mvprintw(x, (i * mngr->col_width) + mngr->col_width -2, "|");
}

/**
 * An alias for ncurses endwin()
 */
void tui_end()
{
	endwin();
}
