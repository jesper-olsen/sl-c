// term_demo.c - manual validation harness for term.h/term.c.
// Bounces a single character across the alternate screen. Press any key,
// or Ctrl-C, to confirm the terminal is restored cleanly on exit.

#include "term.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
        if (!term_init()) {
                fprintf(stderr, "term_init: not a tty or termios setup failed\n");
                return 1;
        }

        term_size size = term_get_size();
        int col = 1;
        int dir = 1;

        while (term_poll_key() == 0) {
                term_move_cursor(size.rows / 2, col);
                term_set_fg_rgb(255, 140, 0);
                printf("X");
                term_reset_color();
                term_flush();
                term_sleep_ms(40);

                term_move_cursor(size.rows / 2, col);
                printf(" ");

                col += dir;
                if (col <= 1 || col >= size.cols) {
                        dir = -dir;
                }
        }

        term_shutdown();
        return 0;
}
