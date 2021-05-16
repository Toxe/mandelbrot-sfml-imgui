# Multi-threaded SFML + ImGui Mandelbrot

A multi-threaded C++ Mandelbrot renderer using SFML + ImGui running at a constant 60 FPS for Windows, Mac and Linux.

This is a graphical, interactive version of my [Mandelbrot Comparison project](https://github.com/Toxe/mandelbrot-comparison) which implements the same Mandelbrot algorithm in 5 different languages (C, C++, Python2/3, PHP and Swift). The colorization of the images is done by using a histogram based smooth coloring method.

## Build

Default build instructions for CMake and Vcpkg. These examples assume that Vcpkg is installed in your home directory. Adjust the paths if necessary.

#### Vcpkg toolchain

Pass your Vcpkg toolchain file via `CMAKE_TOOLCHAIN_FILE`, for example on Windows:  
`-DCMAKE_TOOLCHAIN_FILE=%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake`

Or on Unix systems:  
`-DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake`

### Linux + Mac

```
$ mkdir build
$ cd build
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake ..
$ ninja
$ cd ..
$ ./build/src/mandelbrot
```

### Windows

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
$ cmake --build . --config Release
$ cd ..
$ .\build\src\Release\mandelbrot.exe
```

## Usage

```
A multi-threaded C++ Mandelbrot renderer using SFML + ImGui.
Usage: mandelbrot [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -v                          log level (-v: INFO, -vv: DEBUG, -vvv: TRACE)
  -n,--threads INT:POSITIVE   number of threads (default: number of concurrent threads supported by the system: 24)
  --font-size INT:POSITIVE    UI font size in pixels (default: 22)
  -f,--fullscreen Excludes: --width --height
                              fullscreen (default: false)
  --width INT:POSITIVE Needs: --height Excludes: --fullscreen
                              window width (windowed mode only, default: 2160)
  --height INT:POSITIVE Needs: --width Excludes: --fullscreen
                              window height (windowed mode only, default: 1620)
```
