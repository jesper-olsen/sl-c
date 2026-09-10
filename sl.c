// sl.c - Steam Locomotive (sl) in C23, ported from Masashi Toyoda's original sl. 
#include "term.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------

typedef struct {
        bool accident; // -a          accident - people cry for help
        bool fly;      // -F          fly - train flies
        bool c51;      // -c, --c51   model C51
        bool logo;     // -l, --logo  model SL logo
        bool one;      // -1, --one   start at screen bottom
} sl_config;

static const char *const SL_VERSION = "0.1.0";

static void print_usage(const char *prog)
{
        printf("Steam Locomotive (sl) in C23\n\n");
        printf("Usage: %s [OPTIONS]\n\n", prog);
        printf("Options:\n");
        printf("  -a             Accident - people cry for help\n");
        printf("  -F             Fly - train flies\n");
        printf("  -c, --c51      Model C51\n");
        printf("  -l, --logo     Model SL logo\n");
        printf("  -1, --one      Start at screen bottom\n");
        printf("  -h, --help     Print help\n");
        printf("      --version  Print version\n");
}

// Parses argv into config. On -h/--help or --version, prints and exit(0)s.
// On an unknown flag or -c/-l together, prints to stderr and exit(1)s.
static void sl_config_parse(int argc, char *argv[], sl_config *config)
{
        *config = (sl_config) {};

        static const struct option long_options[] = {
                {"c51", no_argument, nullptr, 'c'},
                {"logo", no_argument, nullptr, 'l'},
                {"one", no_argument, nullptr, '1'},
                {"help", no_argument, nullptr, 'h'},
                {"version", no_argument, nullptr, 'V'},
                {nullptr, 0, nullptr, 0},
        };

        int opt;
        while ((opt = getopt_long(argc, argv, "aFcl1h", long_options, nullptr)) != -1) {
                switch (opt) {
                case 'a':
                        config->accident = true;
                        break;
                case 'F':
                        config->fly = true;
                        break;
                case 'c':
                        config->c51 = true;
                        break;
                case 'l':
                        config->logo = true;
                        break;
                case '1':
                        config->one = true;
                        break;
                case 'h':
                        print_usage(argv[0]);
                        exit(0);
                case 'V':
                        printf("sl %s\n", SL_VERSION);
                        exit(0);
                default:
                        print_usage(argv[0]);
                        exit(1);
                }
        }

        if (config->c51 && config->logo) {
                fprintf(stderr, "error: -c/--c51 conflicts with -l/--logo\n");
                exit(1);
        }
}

// ---------------------------------------------------------------------
// Frame data 
// ---------------------------------------------------------------------

static constexpr int D51_HEIGHT = 10;
static constexpr int D51_FUNNEL = 7;
static constexpr int D51_LENGTH = 83;
static constexpr int D51_PATTERNS = 6;

static const char *const D51_BODY[7] = {
        "      ====        ________                ___________ ",
        "  _D _|  |_______/        \\__I_I_____===__|_________| ",
        "   |(_)---  |   H\\________/ |   |        =|___ ___|   ",
        "   /     |  |   H  |  |     |   |         ||_| |_||   ",
        "  |      |  |   H  |__--------------------| [___] |   ",
        "  | ________|___H__/__|_____/[][]~\\_______|       |   ",
        "  |/ |   |-----------I_____I [][] []  D   |=======|__ ",
};

static const char *const D51_WHEELS[D51_PATTERNS][3] = {
        {
                "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ ",
                " |/-=|___|=    ||    ||    ||    |_____/~\\___/        ",
                "  \\_/      \\O=====O=====O=====O_/      \\_/            ",
        },
        {
                "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ ",
                " |/-=|___|=O=====O=====O=====O   |_____/~\\___/        ",
                "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            ",
        },
        {
                "__/ =| o |=-O=====O=====O=====O \\ ____Y___________|__ ",
                " |/-=|___|=    ||    ||    ||    |_____/~\\___/        ",
                "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            ",
        },
        {
                "__/ =| o |=-~O=====O=====O=====O\\ ____Y___________|__ ",
                " |/-=|___|=    ||    ||    ||    |_____/~\\___/        ",
                "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            ",
        },
        {
                "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ ",
                " |/-=|___|=   O=====O=====O=====O|_____/~\\___/        ",
                "  \\_/      \\__/  \\__/  \\__/  \\__/      \\_/            ",
        },
        {
                "__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__ ",
                " |/-=|___|=    ||    ||    ||    |_____/~\\___/        ",
                "  \\_/      \\_O=====O=====O=====O/      \\_/            ",
        },
};

static const char *const D51_COAL[11] = {
        "                              ",
        "                              ",
        "    _________________         ",
        "   _|                \\_____A  ",
        " =|                        |  ",
        " -|                        |  ",
        "__|________________________|_ ",
        "|__________________________|_ ",
        "   |_D__D__D_|  |_D__D__D_|   ",
        "    \\_/   \\_/    \\_/   \\_/    ",
        "                              ",
};

static constexpr int C51_HEIGHT = 11;
static constexpr int C51_FUNNEL = 7;
static constexpr int C51_LENGTH = 87;
static constexpr int C51_PATTERNS = 6;

static const char *const C51_COAL[12] = {
        "                              ",
        "                              ",
        "                              ",
        "    _________________         ",
        "   _|                \\_____A  ",
        " =|                        |  ",
        " -|                        |  ",
        "__|________________________|_ ",
        "|__________________________|_ ",
        "   |_D__D__D_|  |_D__D__D_|   ",
        "    \\_/   \\_/    \\_/   \\_/    ",
        "                              ",
};

static const char *const MAN_FRAMES[2][2] = {
        { "", "(O)" },
        { "Help!", "\\O/" },
};

static constexpr int LOGO_HEIGHT = 6;
static constexpr int LOGO_FUNNEL = 4;
static constexpr int LOGO_LENGTH = 84;
static constexpr int LOGO_PATTERNS = 6;

static const char *const LOGO_BODY[4] = {
        "     ++      +------ ",
        "     ||      |+-+ |  ",
        "   /---------|| | |  ",
        "  + ========  +-+ |  ",
};

static const char *const LOGO_WHEELS[LOGO_PATTERNS][2] = {
        { " _|--O========O~\\-+  ", "//// \\_/      \\_/    " },
        { " _|--/O========O\\-+  ", "//// \\_/      \\_/    " },
        { " _|--/~O========O-+  ", "//// \\_/      \\_/    " },
        { " _|--/~\\------/~\\-+  ", "//// \\_O========O    " },
        { " _|--/~\\------/~\\-+  ", "//// \\O========O/    " },
        { " _|--/~\\------/~\\-+  ", "//// O========O_/    " },
};

static const char *const LOGO_ERASER = "                     ";

static const char *const LOGO_COAL[7] = {
        "____                 ",
        "|   \\@@@@@@@@@@@     ",
        "|    \\@@@@@@@@@@@@@_ ",
        "|                  | ",
        "|__________________| ",
        "   (O)       (O)     ",
        "                     ",
};

static const char *const LOGO_CAR[7] = {
        "____________________ ",
        "|  ___ ___ ___ ___ | ",
        "|  |_| |_| |_| |_| | ",
        "|__________________| ",
        "|__________________| ",
        "   (O)        (O)    ",
        "                     ",
};

static const char *const C51_BODY[7] = {
        "        ___                                            ",
        "       _|_|_  _     __       __             ___________",
        "    D__/   \\_(_)___|  |__H__|  |_____I_Ii_()|_________|",
        "     | `---'   |:: `--'  H  `--'         |  |___ ___|  ",
        "    +|~~~~~~~~++::~~~~~~~H~~+=====+~~~~~~|~~||_| |_||  ",
        "    ||        | ::       H  +=====+      |  |::  ...|  ",
        "|    | _______|_::-----------------[][]-----|       |  ",
};

static const char *const C51_WHEELS[C51_PATTERNS][4] = {
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|==[]=-     ||      ||      |  ||=======_|__",
                "/~\\____|___|/~\\_|   O=======O=======O  |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|===[]=-    ||      ||      |  ||=======_|__",
                "/~\\____|___|/~\\_|    O=======O=======O |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|===[]=- O=======O=======O  |  ||=======_|__",
                "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|==[]=- O=======O=======O   |  ||=======_|__",
                "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|=[]=- O=======O=======O    |  ||=======_|__",
                "/~\\____|___|/~\\_|      ||      ||      |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
        {
                "| /~~ ||   |-----/~~~~\\  /[I_____I][][] --|||_______|__",
                "------'|oOo|=[]=-      ||      ||      |  ||=======_|__",
                "/~\\____|___|/~\\_|  O=======O=======O   |__|+-/~\\_|     ",
                "\\_/         \\_/  \\____/  \\____/  \\____/      \\_/       ",
        },
};

static const char *const WHEELS_ERASER =
        "                                                       ";

static constexpr int SMOKE_PATTERNS = 16;

// SMOKE[] is indexed by kind, so PUFF_BLACK/PUFF_WHITE must stay 0/1.
enum puff_kind {
        PUFF_BLACK = 0,
        PUFF_WHITE = 1,
};

static const char *const SMOKE[2][SMOKE_PATTERNS] = {
        {
                "(   )", "(    )", "(    )", "(   )", "(  )", "(  )", "( )", "( )",
                "()", "()", "O", "O", "O", "O", "O", " ",
        },
        {
                "(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)", "(@@)", "(@)", "(@)",
                "@@", "@@", "@", "@", "@", "@", "@", " ",
        },
};

static const char *const SMOKE_ERASER[SMOKE_PATTERNS] = {
        "     ", "      ", "      ", "     ", "    ", "    ", "   ", "   ",
        "  ", "  ", " ", " ", " ", " ", " ", " ",
};

static const int SMOKE_DY[SMOKE_PATTERNS] = { 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int SMOKE_DX[SMOKE_PATTERNS] = { -2, -1, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3 };

static enum puff_kind puff_kind_succ(enum puff_kind kind)
{
        return (kind == PUFF_BLACK) ? PUFF_WHITE : PUFF_BLACK;
}

// ---------------------------------------------------------------------
// Tui + smoke plume + per-locomotive rendering
// ---------------------------------------------------------------------

struct tui {
        int rows;
        int cols;
};

static constexpr int SMOKE_PLUME_CAPACITY = 256;

struct smoke_puff {
        int y;
        int x;
        int frame;
        enum puff_kind kind;
};

struct smoke_plume {
        struct smoke_puff puffs[SMOKE_PLUME_CAPACITY];
        int head;
        int count;
        enum puff_kind next_puff;
};

// Initialises the terminal (via term_init) and captures its current size.
// Returns false if term_init() fails (not a tty, termios setup failed).
[[nodiscard]] static bool tui_init(struct tui *t)
{
        if (!term_init()) {
                return false;
        }
        term_size size = term_get_size();
        *t = (struct tui) {
                .rows = size.rows, .cols = size.cols
        };
        return true;
}

// Pre-allocated, fixed capacity
[[nodiscard]] static struct smoke_plume smoke_plume_init(void)
{
        return (struct smoke_plume) {
                .head = 0,
                .count = 0,
                .next_puff = PUFF_WHITE,
        };
}

// Bounds-clipped text draw: skips text that's fully off-screen, clips text
// that's partially off-screen at either horizontal edge. 
static void tui_draw_text(struct tui *t, int y, int x, const char *s)
{
        int len = (int)strlen(s);
        if (y < 0 || y >= t->rows || x >= t->cols || x + len <= 0) {
                return;
        }
        int start = (x < 0) ? -x : 0;
        int end = (x + len > t->cols) ? (t->cols - x) : len;
        if (start < end) {
                term_move_cursor(y + 1, (x < 0 ? 0 : x) + 1); // term_move_cursor is 1-based
                printf("%.*s", end - start, s + start);
        }
}

static void tui_draw_lines(struct tui *t, int y, int x, const char *const *lines, int count)
{
        for (int i = 0; i < count; i++) {
                tui_draw_text(t, y + i, x, lines[i]);
        }
}

static void tui_add_man(struct tui *t, int y, int x, int train_length)
{
        int idx = ((train_length + x) / 12) % 2;
        for (int i = 0; i < 2; i++) {
                tui_draw_text(t, y + i, x, MAN_FRAMES[idx][i]);
        }
}

static void smoke_plume_add(struct smoke_plume *plume, struct tui *t, int y, int x)
{
        if (x % 4 != 0) {
                return;
        }

        for (int i = 0; i < plume->count; i++) {
                struct smoke_puff *puff = &plume->puffs[(plume->head + i) % SMOKE_PLUME_CAPACITY];
                tui_draw_text(t, puff->y, puff->x, SMOKE_ERASER[puff->frame]);
                puff->y -= SMOKE_DY[puff->frame];
                puff->x += SMOKE_DX[puff->frame];
                if (puff->frame < SMOKE_PATTERNS - 1) {
                        puff->frame++;
                }
                tui_draw_text(t, puff->y, puff->x, SMOKE[puff->kind][puff->frame]);
        }

        enum puff_kind kind = plume->next_puff;
        tui_draw_text(t, y, x, SMOKE[kind][0]);

        int idx;
        if (plume->count < SMOKE_PLUME_CAPACITY) {
                idx = (plume->head + plume->count) % SMOKE_PLUME_CAPACITY;
                plume->count++;
        } else {
                // At capacity: overwrite the oldest puff in place 
                idx = plume->head;
                plume->head = (plume->head + 1) % SMOKE_PLUME_CAPACITY;
        }
        plume->puffs[idx] = (struct smoke_puff) {
                .y = y, .x = x, .frame = 0, .kind = kind
        };

        plume->next_puff = puff_kind_succ(plume->next_puff);
}

static bool tui_render_d51(struct tui *t, struct smoke_plume *plume,
                           const sl_config *cfg, int x)
{
        if (x < -D51_LENGTH) {
                return false;
        }
        int y = cfg->one ? t->rows - D51_HEIGHT : t->rows / 2 - 5;
        int dy = 0;
        if (cfg->fly) {
                y = (x / 7) + t->rows - (t->cols / 7) - D51_HEIGHT;
                dy = 1;
        }
        int ptn = (D51_LENGTH + x) % D51_PATTERNS;

        tui_draw_lines(t, y, x, D51_BODY, 7);
        int height = 7;
        tui_draw_lines(t, y + height, x, D51_WHEELS[ptn], 3);
        height += 3;
        tui_draw_text(t, y + height, x, WHEELS_ERASER);

        constexpr int car_x_offset = 53;
        tui_draw_lines(t, y + dy, x + car_x_offset, D51_COAL, 11);

        if (cfg->accident) {
                tui_add_man(t, y + 2, x + 43, D51_LENGTH);
                tui_add_man(t, y + 2, x + 47, D51_LENGTH);
        }

        smoke_plume_add(plume, t, y - 1, x + D51_FUNNEL);
        return true;
}

static bool tui_render_c51(struct tui *t, struct smoke_plume *plume,
                           const sl_config *cfg, int x)
{
        if (x < -C51_LENGTH) {
                return false;
        }
        int y = cfg->one ? t->rows - C51_HEIGHT : t->rows / 2 - 5;
        int dy = 0;
        if (cfg->fly) {
                y = (x / 7) + t->rows - (t->cols / 7) - C51_HEIGHT;
                dy = 1;
        }
        int ptn = (C51_LENGTH + x) % C51_PATTERNS;

        tui_draw_lines(t, y, x, C51_BODY, 7);
        int height = 7;
        tui_draw_lines(t, y + height, x, C51_WHEELS[ptn], 4);
        height += 4;
        tui_draw_text(t, y + height, x, WHEELS_ERASER);

        constexpr int car_x_offset = 55;
        tui_draw_lines(t, y + dy, x + car_x_offset, C51_COAL, 12);

        if (cfg->accident) {
                tui_add_man(t, y + 3, x + 45, C51_LENGTH);
                tui_add_man(t, y + 3, x + 49, C51_LENGTH);
        }

        smoke_plume_add(plume, t, y - 1, x + C51_FUNNEL);
        return true;
}

static bool tui_render_logo(struct tui *t, struct smoke_plume *plume,
                            const sl_config *cfg, int x)
{
        if (x < -LOGO_LENGTH) {
                return false;
        }
        int y = cfg->one ? t->rows - LOGO_HEIGHT : t->rows / 2 - 3;
        int py1 = 0, py2 = 0, py3 = 0;
        if (cfg->fly) {
                y = (x / 6) + t->rows - (t->cols / 6) - LOGO_HEIGHT;
                py1 = 2;
                py2 = 4;
                py3 = 6;
        }
        int ptn = ((LOGO_LENGTH + x) / 3) % LOGO_PATTERNS;

        tui_draw_lines(t, y, x, LOGO_BODY, 4);
        int height = 4;
        tui_draw_lines(t, y + height, x, LOGO_WHEELS[ptn], 2);
        height += 2;
        tui_draw_text(t, y + height, x, LOGO_ERASER);

        tui_draw_lines(t, y + py1, x + 21, LOGO_COAL, 7);
        tui_draw_lines(t, y + py2, x + 42, LOGO_CAR, 7);
        tui_draw_lines(t, y + py3, x + 63, LOGO_CAR, 7);

        if (cfg->accident) {
                tui_add_man(t, y + 1, x + 14, LOGO_LENGTH);
                tui_add_man(t, y + 1 + py2, x + 45, LOGO_LENGTH);
                tui_add_man(t, y + 1 + py2, x + 53, LOGO_LENGTH);
                tui_add_man(t, y + 1 + py3, x + 66, LOGO_LENGTH);
                tui_add_man(t, y + 1 + py3, x + 74, LOGO_LENGTH);
        }

        smoke_plume_add(plume, t, y - 1, x + LOGO_FUNNEL);
        return true;
}

// Draws the selected locomotive (and its smoke) at horizontal offset x.
// Returns false once the train has fully scrolled off-screen - the caller
// should stop advancing x once this returns false.
[[nodiscard]] static bool tui_render_train(struct tui *t, struct smoke_plume *plume,
                const sl_config *cfg, int x)
{
        bool visible;
        if (cfg->logo) {
                visible = tui_render_logo(t, plume, cfg, x);
        } else if (cfg->c51) {
                visible = tui_render_c51(t, plume, cfg, x);
        } else {
                visible = tui_render_d51(t, plume, cfg, x);
        }
        term_flush();
        return visible;
}

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------

int main(int argc, char *argv[])
{
        sl_config cfg;
        sl_config_parse(argc, argv, &cfg);

        struct tui tui;
        if (!tui_init(&tui)) {
                fprintf(stderr, "sl: not a tty, or terminal setup failed\n");
                return 1;
        }

        struct smoke_plume plume = smoke_plume_init();
        int x = tui.cols - 1;

        while (tui_render_train(&tui, &plume, &cfg, x)) {
                term_sleep_ms(40);
                x--;
        }

        term_shutdown();
        return 0;
}
