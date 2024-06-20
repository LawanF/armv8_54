#include "window.h"
#include <ncurses.h>
#include <stdlib.h>

static WINDOW *terminal = NULL;

void init_window(void) {
    terminal = initscr();
    if (terminal == NULL) exit(EXIT_FAILURE);
    nodelay(terminal, TRUE);
    printw("Hello world!");
    wgetch(terminal);
}

void end_window(void) {
    endwin();
}
