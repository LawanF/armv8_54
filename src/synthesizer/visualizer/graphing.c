#include <ncurses.h>
#include <stdlib.h>

/*
 * A function that takes in
 *  - an array of frequencies
 *  - an array of amplitudes
 *  - the window the graph will be displayed on
 *  - the width of the window for the graph
 *  - the height of the window for the graph
 *  - the size of the arrays
 */

void create_graph(float *frequencies, float *amplitudes, WINDOW *win, int graph_x, int graph_y, int size) {
    //  Scale based on the maximum amplitude
    float max_amp = 0;
    for (int i = 0; i < size; i++) {
        if (amplitudes[i] > max_amp) { max_amp = amplitudes[i]; }
    }

    //  Plot the data
    for (int i = 0; i < size; i++) {
        int x = (frequencies[i] * graph_x) / frequencies[size - 1]; // scale frequency to fit width
        int y = (amplitudes[i] * (graph_y - 1)) / max_amp; // scale amplitude to fit height
        mvwprintw(win, graph_y - y - 1, x, "_");
    }

}


/*

// Testing the graphing function

int main(void) {
    // Example data
    float frequencies[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    float amplitudes[] = {3, 6, 1, 7, 2, 9, 5, 8, 4, 10};
    int size = sizeof(frequencies) / sizeof(frequencies[0]);

    // Initialize ncurses
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    // Create the first window

    int first_win_height = LINES;
    int first_win_width = COLS / 2;
    WINDOW *first_win = newwin(first_win_height, first_win_width, 0, 0);
    wbkgd(first_win, COLOR_PAIR(1));
    box(first_win, 0, 0);
    mvwprintw(first_win, 1, 1, "This is the first window");

    // Create the second window for the graph
    int graph_win_height = LINES;
    int graph_win_width = COLS - first_win_width;
    WINDOW *graph_win = newwin(graph_win_height, graph_win_width, 0, first_win_width);
    wbkgd(graph_win, COLOR_PAIR(2));
    box(graph_win, 0, 0);

    // Plot the graph in the second window
    create_graph(frequencies, amplitudes, graph_win, graph_win_width - 2, graph_win_height - 2, size);

    // Refresh both windows
    wrefresh(first_win);
    wrefresh(graph_win);

    while (true) { int z = 0; }

    // Wait for user input
    getch();

    // End ncurses
    endwin();

    return 0;

}

*/
