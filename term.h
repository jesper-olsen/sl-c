// term.h
#ifndef TERM_H
#define TERM_H

typedef struct {
        int rows;
        int cols;
} term_size_t;

// Enable raw mode, switch to the alternate screen buffer, hide the cursor,
// and register exit/signal handlers so the terminal is always restored.
// Returns false if stdin isn't a tty or termios setup fails.
[[nodiscard]] bool term_init(void);

// Restore the terminal to its pre-term_init state: show cursor, leave the
// alternate screen buffer, restore original termios settings. Idempotent -
// registered automatically via atexit and common signal handlers, but can
// also be called directly.
void term_shutdown(void);

// Current terminal size. Falls back to 80x24 if the ioctl fails.
[[nodiscard]] term_size_t term_get_size(void);

void term_clear(void);
void term_move_cursor(int row, int col);   // 1-based, top-left origin
void term_hide_cursor(void);
void term_show_cursor(void);
void term_flush(void);

void term_set_fg_rgb(unsigned char r, unsigned char g, unsigned char b);
void term_reset_color(void);

// Non-blocking: returns the next pending byte from stdin, or 0 if none is
// available. Used to let an animation exit early on any keypress.
int term_poll_key(void);

#endif // TERM_H
