#include "profiler.h"

#include <stdlib.h>

int main(void)
{
    profiler_clock_begin("total");
    profiler_clock_begin("first loop");
    for (size_t i = 0; i < (1 << 20); ++i) {
        rand();
    }
    profiler_clock_end();
    profiler_clock_begin("second loop");
    for (size_t i = 0; i < (1 << 16); ++i) {
        rand();
    }
    profiler_clock_end();
    profiler_clock_end();
    return 0;
}
