// term.c - raw-mode ANSI terminal control for sl.
//
// Deliberately layered directly on termios + VT100/xterm escape sequences
// rather than curses/notcurses: sl only ever blits a single string per
// frame, so a library boundary buys nothing here, and this keeps the C
// version philosophically aligned with the crossterm-based Rust port.

#include "term.h"

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

static struct termios orig_termios;
static bool raw_mode_active = false;

// Fixed escape sequences, written directly with write() rather than
// printf/fputs, so term_shutdown() is callable from the signal handler
// below without touching stdio's internal buffer.
#define TERM_WRITE_LIT(s) ((void)write(STDOUT_FILENO, (s), sizeof(s) - 1))

static void term_restore_termios(void)
{
        if (raw_mode_active) {
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
                raw_mode_active = false;
        }
}

void term_shutdown(void)
{
        // Flush anything printf()'d but not yet written before issuing raw
        // write()s, so cleanup codes can't land ahead of pending frame output.
        fflush(stdout);
        TERM_WRITE_LIT("\x1b[?25h");   // show cursor
        TERM_WRITE_LIT("\x1b[?1049l"); // leave alternate screen buffer
        term_restore_termios();
}

// NOTE: tcsetattr()/fflush() aren't on POSIX's strict async-signal-safe
// list, but every mainstream libc implements tcsetattr as a single ioctl,
// and the process is exiting either way. This is the same pragmatic
// approach tmux/less/vim take, rather than deferring cleanup to a flag
// polled from the main loop.
static void term_signal_handler(int sig)
{
        term_shutdown();
        struct sigaction dfl = {};
        dfl.sa_handler = SIG_DFL;
        sigaction(sig, &dfl, nullptr);
        raise(sig);
}

static void term_install_handlers(void)
{
        atexit(term_shutdown);

        struct sigaction sa = {};
        sa.sa_handler = term_signal_handler;
        sigemptyset(&sa.sa_mask);

        const int sigs[] = {SIGINT, SIGTERM, SIGHUP, SIGQUIT};
        for (size_t i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++) {
                sigaction(sigs[i], &sa, nullptr);
        }
}

[[nodiscard]] bool term_init(void)
{
        if (!isatty(STDIN_FILENO)) {
                return false;
        }
        if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
                return false;
        }

        term_install_handlers();

        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);   // no echo, read byte-by-byte
        raw.c_iflag &= ~(IXON | ICRNL);    // no flow control, no CR->NL
        raw.c_cc[VMIN] = 0;                // reads return immediately...
        raw.c_cc[VTIME] = 0;               // ...even with zero bytes available

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
                return false;
        }
        raw_mode_active = true;

        TERM_WRITE_LIT("\x1b[?1049h"); // enter alternate screen buffer
        TERM_WRITE_LIT("\x1b[?25l");   // hide cursor
        term_clear();
        term_flush();
        return true;
}

[[nodiscard]] term_size_t term_get_size(void)
{
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
                return (term_size_t) {
                        .rows = 24, .cols = 80
                };
        }
        return (term_size_t) {
                .rows = ws.ws_row, .cols = ws.ws_col
        };
}

void term_clear(void)
{
        TERM_WRITE_LIT("\x1b[2J\x1b[H");
}

void term_move_cursor(int row, int col)
{
        printf("\x1b[%d;%dH", row, col);
}

void term_hide_cursor(void)
{
        TERM_WRITE_LIT("\x1b[?25l");
}

void term_show_cursor(void)
{
        TERM_WRITE_LIT("\x1b[?25h");
}

void term_flush(void)
{
        fflush(stdout);
}

void term_set_fg_rgb(unsigned char r, unsigned char g, unsigned char b)
{
        printf("\x1b[38;2;%u;%u;%um", r, g, b);
}

void term_reset_color(void)
{
        TERM_WRITE_LIT("\x1b[0m");
}

int term_poll_key(void)
{
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        return (n == 1) ? (int)c : 0;
}
