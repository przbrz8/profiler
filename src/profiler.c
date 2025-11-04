#include "profiler.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define _PROFILER_NAME "profiler"

typedef struct {
    const char* label;
    struct timespec begin;
    struct timespec end;
} Profiler_Clock;

typedef struct {
    Profiler_Clock* items;
    size_t count;
    size_t capacity;
} Profiler_Clock_Stack;

static void _profiler_clock_stack_add(void);
static void _profiler_clock_stack_free(void);
static void _profiler_clock_stack_remove(void);

static Profiler_Clock_Stack _clock_stack = {0};

#define _PROFILER_CLOCK_STACK_CAPACITY_INIT 8
#define _PROFILER_CLOCK_STACK_CAPACITY_GROW 2
static void _profiler_clock_stack_add(void)
{
    if (_clock_stack.capacity <= _clock_stack.count) {
        if (_clock_stack.capacity == 0) {
            _clock_stack.capacity = _PROFILER_CLOCK_STACK_CAPACITY_INIT;
        }
        else {
            _clock_stack.capacity *= _PROFILER_CLOCK_STACK_CAPACITY_GROW;
        }
        Profiler_Clock* items_temp = (Profiler_Clock*)realloc(_clock_stack.items, sizeof(Profiler_Clock) * _clock_stack.capacity);
        if (!items_temp) {
            fprintf(stderr, "%s: failed to grow clock stack\n", _PROFILER_NAME);
            exit(EXIT_FAILURE);
        }
        _clock_stack.items = items_temp;
    }
    Profiler_Clock clock = {0};
    _clock_stack.items[_clock_stack.count++] = clock;
}

