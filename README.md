# sl (Steam Locomotive)

A C23 port of Masashi Toyoda's classic [sl](https://github.com/mtoyoda/sl) command-line utility.

This version removes the legacy `libcurses` dependency for a lighter footprint and introduces a new option (`-1`) to start the train at the bottom of the screen.

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

## Documentation

A manual page is included. You can view it locally using:

```bash
man ./sl.1
```
