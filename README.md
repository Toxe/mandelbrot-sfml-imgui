# Multi-threaded SFML + ImGui Mandelbrot

A non-blocking, multi-threaded C++ Mandelbrot renderer using SFML + ImGui running at a constant 60 FPS.

This is a graphical, interactive version of my [Mandelbrot Comparison project](https://github.com/Toxe/mandelbrot-comparison) which implements the same Mandelbrot algorithm in 5 different languages (C, C++, Python2/3, PHP and Swift).

## Build

Default build instructions for CMake and Vcpkg.

Pass your Vcpkg toolchain file via `CMAKE_TOOLCHAIN_FILE`, for example `-DCMAKE_TOOLCHAIN_FILE=%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake`
or `-DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake`.

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

Pass your Vcpkg toolchain file via `CMAKE_TOOLCHAIN_FILE`, for example: `-DCMAKE_TOOLCHAIN_FILE=%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake`.

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
  -f,--fullscreen             fullscreen (default: false)
  -n,--threads INT            number of threads (default: number of concurrent threads supported by the system: 24)
  --font-size INT             UI font size in pixels
```
