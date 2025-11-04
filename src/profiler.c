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

#define _PROFILER_CLOCK_STACK_CAPACITY_INCREMENT 8
static void _profiler_clock_stack_add(void)
{
    if (_clock_stack.capacity == _clock_stack.count) {
        size_t new_capacity = _clock_stack.capacity + _PROFILER_CLOCK_STACK_CAPACITY_INCREMENT;
        Profiler_Clock* new_items = (Profiler_Clock*)realloc(_clock_stack.items, sizeof(*new_items) * new_capacity);
        if (!new_items) {
            fprintf(stderr, "%s: failed to grow clock stack\n", _PROFILER_NAME);
            _profiler_clock_stack_free();
            exit(EXIT_FAILURE);
        }
        _clock_stack.items = new_items;
        _clock_stack.capacity = new_capacity;
    }
    _clock_stack.items[_clock_stack.count++] = (Profiler_Clock){0};
}

static void _profiler_clock_stack_free(void)
{
    if (_clock_stack.items == NULL) {
        return;
    }
    free(_clock_stack.items);
    _clock_stack = (Profiler_Clock_Stack){0};
}

static void _profiler_clock_stack_remove(void)
{
    if (_clock_stack.count == 0) {
        return;
    }
    Profiler_Clock* clock = &_clock_stack.items[--_clock_stack.count];
    clock->label = NULL;
    clock->begin = (struct timespec){0};
    clock->end = (struct timespec){0};
    if (_clock_stack.count == 0) {
        _profiler_clock_stack_free();
    }
}

void profiler_clock_begin(const char* clock_label)
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0) {
        fprintf(stderr, "%s: error: failed to get current monotonic time: %s\n", _PROFILER_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    _profiler_clock_stack_add();
    Profiler_Clock* clock = &_clock_stack.items[_clock_stack.count - 1];
    clock->label = clock_label;
    clock->begin = tp;
}

double profiler_clock_end(void)
{
    if (_clock_stack.count == 0) {
        fprintf(stderr, "%s: error: failed to match 'clock_end' to 'clock_begin'\n", _PROFILER_NAME);
        return 0.0;
    }
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0) {
        fprintf(stderr, "%s: error: failed to get current monotonic time: %s\n", _PROFILER_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    Profiler_Clock* clock = &_clock_stack.items[_clock_stack.count - 1];
    clock->end = tp;
    double time = (clock->end.tv_sec - clock->begin.tv_sec) + (clock->end.tv_nsec - clock->begin.tv_nsec) * 1.0e-9;
    fprintf(stderr, "%s: elapsed: %.9lf[s] on clock '", _PROFILER_NAME, time);
    for (size_t i = 0; i < _clock_stack.count; ++i) {
        fprintf(stderr, "%s", _clock_stack.items[i].label);
        if (i < _clock_stack.count - 1) {
            fprintf(stderr, ".");
        }
    }
    fprintf(stderr, "'\n");
    _profiler_clock_stack_remove();
    return time;
}

