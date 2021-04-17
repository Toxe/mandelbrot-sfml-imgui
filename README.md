# Multi-threaded SFML + ImGui Mandelbrot

A multi-threaded C++ Mandelbrot renderer using SFML + ImGui.

## Usage

```
DESCRIPTION
    A multi-threaded C++ Mandelbrot renderer using SFML + ImGui.

SYNOPSIS
        mandelbrot [-h] [-v|-vv] [-f] [-n <num_threads>] [--font-size <font_size>]

OPTIONS
        -h, --help  show help
        -v, --verbose
                    show info log messages (verbose)

        -vv, --debug
                    show debug log messages (very verbose)

        -f, --fullscreen
                    fullscreen (default: false)

        -n, --threads <num_threads>
                    number of threads (default: number of concurrent threads supported by the
                    system: 4)

        --font-size <font_size>
                    UI font size in pixels
```
