CC = clang
CFLAGS = -Wall -O3 -std=c23 -ffast-math -march=native -DNDEBUG
#CFLAGS = -Wall -O0 -std=c23 -g -fsanitize=address -fsanitize=thread
LDFLAGS = -lm

TERM_DEMO_SRC := term.c term_demo.c
SL_SRC        := term.c sl.c
HEADERS       := term.h

ASTYLE_OPTS := --style=kr \
               --indent=spaces=8 \
               --pad-oper \
               --pad-comma \
               --unpad-brackets \
               --squeeze-ws

.PHONY: all clean fmt

all: term_demo sl

term_demo: $(TERM_DEMO_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(TERM_DEMO_SRC) $(LDFLAGS)

sl: $(SL_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SL_SRC) $(LDFLAGS)

clean:
	rm -f term_demo sl

fmt:
	astyle $(ASTYLE_OPTS) $(TERM_DEMO_SRC) $(SL_SRC) $(HEADERS)
