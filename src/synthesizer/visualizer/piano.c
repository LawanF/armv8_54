#include <ncurses.h>
#include <stdlib.h>
#include "../headers/keyboard.h"

#define WHITE_KEYS 15
#define BLACK_KEYS 10
#define PIANO_MARGIN 2

static bool is_white[] = {
    true, false, true, true, false, true, false, true, true, false, true, false, // First octave.
    true, false, true, true, false, true, false, true, true, false, true, false, // Second octave.
    true // High A.
};

static void generate_key(WINDOW *win, int x, int width, int height) {
    for (int y = PIANO_MARGIN; y < PIANO_MARGIN + height; y++) {
        for (int w = 0; w < width; w++) {
            mvwprintw(win, y, x + w, " ");
        }
    }
}

void draw_piano(WINDOW *win) {
    // Set piano key constants
    int white_key_width = COLS / (WHITE_KEYS + BLACK_KEYS);
    int white_key_height = LINES / 4; // decided arbitrarily, can change if too big
    int black_key_width = white_key_width / 2;
    int black_key_height = white_key_height / 2;
    int black_key_offset = white_key_width - black_key_width / 2;

    // Draw the keys
    init_pair(1, COLOR_WHITE, COLOR_WHITE); // white keys
    init_pair(2, COLOR_BLACK, COLOR_BLACK); // black keys
    init_pair(3, COLOR_YELLOW, COLOR_YELLOW); // selection colour
    for (int i = 0; i < WHITE_KEYS + BLACK_KEYS; i++) {
        int x = PIANO_MARGIN + i * white_key_width - (is_white[i] ? 0 : black_key_offset);
        int color_pair = (get_note_on(i) == ON) ? 3
                                                : (is_white[i] ? 1 : 2);
        wattron(win, COLOR_PAIR(color_pair)); // adds attributes i.e. applies colour

        if (is_white[i]) {
            generate_key(win, x, white_key_width, white_key_height);
        } else {
            generate_key(win, x, black_key_width, black_key_height);
        }

        wattroff(win, COLOR_PAIR(color_pair));
    }

}
