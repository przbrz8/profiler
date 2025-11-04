#ifndef PROFILER_H
#define PROFILER_H

#include <stddef.h>

typedef enum {
    SECONDS,
    MILLISECONDS,
    NANOSECONDS
} Profiler_Units;

void profiler_clock_begin(const char* clock_label);
void profiler_clock_end(void);
void profiler_output(const Profiler_Units unit);

#endif // PROFILER_H

