# Profiler
A lightweight tool for measuring execution time of C code blocks.
---
## Installation
To install profiler system-wide for linux simply run:
```
$ cc -o install install.c
$ ./install
```
---
## Features
- Simple API
- Nested clocks
- Lightweight
---
## Usage
Using this library is really simple
```c
#include "profiler.h"

#include <stdlib.h>

int main(void)
{
    profiler_clock_begin("total");
    profiler_clock_begin("first loop");
    for (size_t i = 0; i < 10000000; ++i) {
        rand();
    }
    profiler_clock_end();
    profiler_clock_begin("second loop");
    for (size_t i = 0; i < 1000000; ++i) {
        rand();
    }
    profiler_clock_end();
    profiler_clock_end();
    profiler_output(SECONDS);
    return 0;
}
```
To link with this library use `-lprof` flag in the compilation command
```
$ cc -o main main.c -lprof
```



