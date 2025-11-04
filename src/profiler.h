#ifndef PROFILER_H
#define PROFILER_H

#include <stddef.h>

typedef enum {
    PROFILER_S,
    PROFILER_MS,
    PROFILER_NS
} Profiler_Output_Unit;

void profiler_clock_begin(const char* clock_label);
void profiler_clock_end(void);
void profiler_output(const Profiler_Output_Unit unit);

#endif // PROFILER_H

