CC = clang
CFLAGS = -Wall -O3 -std=c23 -ffast-math -march=native -DNDEBUG
#CFLAGS = -Wall -O0 -std=c23 -g -fsanitize=address -fsanitize=thread
LDFLAGS = -lm

TARGETS := term_demo

SRC     := term.c term_demo.c
HEADER  := term.h

ASTYLE_OPTS := --style=kr \
               --indent=spaces=8 \
               --pad-oper \
               --pad-comma \
               --unpad-brackets \
               --squeeze-ws

.PHONY: all clean fmt

all: $(TARGETS)

$(TARGETS): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGETS)

fmt:
	astyle $(ASTYLE_OPTS) $(SRC) $(HEADER)
