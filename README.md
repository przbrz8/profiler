# Profiler
A lightweight tool for measuring execution time of C code blocks.
---
## Installation
To install profiler system-wide for linux simply run:
```
$ cc -o cbc cbc.c
$ sudo ./cbc
```
---
## Features
- Simple API consisting of only 2 functions
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
    return 0;
}
```



