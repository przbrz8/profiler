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

