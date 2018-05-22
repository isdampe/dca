#include <ncurses.h>
#include "tui.h"

int main(int argc, char *argv[])
{	

	tui_mngr mngr = tui_create_mgr(3);
	tui_print_borders(&mngr);
	tui_print_col(&mngr, 1, 1, "Testing line 1adjksahdk ajshdak sjdhaksjhd akjsdh");
	tui_print_col(&mngr, 1, 2, "Testing line 2");
	tui_print_col(&mngr, 2, 1, "Second column");

	int ch;
	while((ch = getch()) != KEY_F(1))
	{	
		if (ch == 'q' || ch == 'Q')
			break;
	}

	endwin();
	return 0;
}
