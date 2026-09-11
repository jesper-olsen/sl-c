# sl (Steam Locomotive)

A C23 port of Masashi Toyoda's classic [sl](https://github.com/mtoyoda/sl) command-line utility.

This version removes the legacy `libncurses` dependency and introduces a new option (`-1`) to start the train at the bottom of the screen.


## Prerequisites

You will need the following installed:

1. A C compiler (e.g., GCC or Clang)
2. Make 

Clone the repository and build the program:

```bash
git clone https://github.com/jesper-olsen/sl-c.git
cd sl-c
make
``` 

## Run

```bash
 ./sl -h
Steam Locomotive (sl) in C23

Usage: ./sl [OPTIONS]

Options:
  -a             Accident - people cry for help
  -F             Fly - train flies
  -c, --c51      Model C51
  -l, --logo     Model SL logo
  -1, --one      Start at screen bottom
  -h, --help     Print help
      --version  Print version
```

```
./sl

```

![Gif animation of the sl command](demo.gif)


## Size & Performance

Comparison of 
1. sl-c (C23) 
2. Masashi Toyoda's original (C89, ncurses) 
3. [rust port](https://github.com/jesper-olsen/sl) built on the [crossterm](https://docs.rs/crossterm/latest/crossterm/) crate. 

Two runs of each, measured with `/usr/bin/time -l` on a Macbook Air M5 (identical terminal window size):

|                       | sl-c     | Toyoda's original | Rust port    |
|---                    |---:      |---:               |---:          |
| Executable size       | 35,536 B | 35,104 B          | 1,105,408 B  |
| Peak RSS              | 1.31 MB  | 2.19 MB           | 1.89 MB      |
| Peak memory footprint | 944 KB   | 1.75 MB           | 1.01 MB      |
| Instructions retired  | ~44.0M   | ~321.8M           | ~44.3M       |
| Cycles elapsed        | ~30.8M   | ~114.5M           | ~31.7M       |

Notes:

- **On disk**, sl-c and Toyoda's original are both small, for different reasons. Toyoda's links `ncurses` dynamically (`-lncurses`), so its terminal-handling code is not part of the executable. sl-c has no terminal library at all - `term.c` is ~100 lines of termios and ANSI escapes. The Rust binary is far larger mainly because Rust statically links its dependencies; of those, `clap`'s derive-based argument parsing accounts for most of the size.
- **At runtime**: Toyoda's version has the largest memory footprint of the three, because ncurses gets mapped into the process. It also retires roughly 7x more CPU instructions than sl-c or the Rust port for the same on-screen animation - ncurses diffs its "virtual" screen against the "physical" one on every refresh to minimise terminal writes, which is useful work for programs with mostly-static screens, and pure overhead here, since `sl` repaints its whole visible frame every tick regardless.
- **Wall-clock time**:  Similar across all three (~10.5-11.3s for a full screen-width scroll), governed by the same ~40ms-per-frame sleep loop.


## Documentation

A manual page is included. You can view it locally using:

```bash
man ./sl.1
```
