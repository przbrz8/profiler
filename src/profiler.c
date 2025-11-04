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

typedef struct {
    double time;
    char* label;
} Profiler_Output;

typedef struct {
    Profiler_Output* items;
    size_t count;
    size_t capacity;
} Profiler_Output_List;

static void _profiler_clock_stack_add(void);
static void _profiler_clock_stack_free(void);
static void _profiler_clock_stack_remove(void);

static void _profiler_output_list_add(void);
static void _profiler_output_list_free(void);
static void _profiler_output_list_print(size_t index, size_t max_len);

static Profiler_Clock_Stack _clock_stack = {0};
static Profiler_Output_List _output_list = {0};

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

#define _PROFILER_OUTPUT_LIST_CAPACITY_INCREMENT 8
static void _profiler_output_list_add(void)
{
    if (_output_list.capacity == _output_list.count) {
        size_t new_capacity = _output_list.capacity + _PROFILER_OUTPUT_LIST_CAPACITY_INCREMENT;
        Profiler_Output* new_items = (Profiler_Output*)realloc(_output_list.items, sizeof(*new_items) * new_capacity);
        if (!new_items) {
            fprintf(stderr, "%s: failed to grow output list\n", _PROFILER_NAME);
            _profiler_output_list_free();
            exit(EXIT_FAILURE);
        }
        _output_list.items = new_items;
        _output_list.capacity = new_capacity;
    }
    _output_list.items[_output_list.count++] = (Profiler_Output){0};
}

static void _profiler_output_list_free(void)
{
    if (_output_list.items == NULL) {
        return;
    }
    free(_output_list.items);
    _output_list = (Profiler_Output_List){0};
}

#define _PROFILER_OUTPUT_SPACING 2
static void _profiler_output_list_print(const size_t index, const size_t max_len)
{
    Profiler_Output* output = &_output_list.items[_output_list.count - 1];
    size_t len = (max_len + 2 + _PROFILER_OUTPUT_SPACING);
    char* path = (char*)malloc(sizeof(*path) * (len + 1));
    if (!path) {
        fprintf(stderr, "%s: error: failed to allocate memory for text output\n", _PROFILER_NAME);
    }
    path[len] = '\0';
    fprintf(stderr, "clock: ");
    size_t pos = 0;
    for (size_t i = 0; i < len; ++i) {
        path[i] = '.';
    }
    path[pos++] = '\'';
    strcpy(path + pos, _output_list.items[index].label);
    pos += strlen(_output_list.items[index].label);
    path[pos++] = '\'';
    fprintf(stderr, "%s%.9lf s\n", path, _output_list.items[index].time);
    free(path);
}

void profiler_clock_begin(const char* clock_label)
{
    if (strlen(clock_label) == 0) {
        fprintf(stderr, "%s: error: invalid clock label\n", _PROFILER_NAME);
        return;
    }
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

void profiler_clock_end(void)
{
    if (_clock_stack.count == 0) {
        fprintf(stderr, "%s: error: failed to match 'clock_end' to 'clock_begin'\n", _PROFILER_NAME);
        return;
    }
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0) {
        fprintf(stderr, "%s: error: failed to get current monotonic time: %s\n", _PROFILER_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    Profiler_Clock* clock = &_clock_stack.items[_clock_stack.count - 1];
    clock->end = tp;
    _profiler_output_list_add();
    size_t len = _clock_stack.count - 1;
    for (size_t i = 0; i < _clock_stack.count; ++i) {
        len += strlen(_clock_stack.items[i].label);
    }
    char* label = (char*)malloc(sizeof(*label) * (len + 1));
    if (!label) {
        fprintf(stderr, "%s: error: failed to allocate memory for output label\n", _PROFILER_NAME);
        exit(EXIT_FAILURE);
    }
    size_t pos = 0;
    for (size_t i = 0; i < _clock_stack.count; ++i) {
        strcpy(label + pos, _clock_stack.items[i].label);
        pos += strlen(_clock_stack.items[i].label);
        if (i < _clock_stack.count - 1) {
            label[pos++] = '.';
        }
    }
    _output_list.items[_output_list.count - 1].label = label;
    double time = (clock->end.tv_sec - clock->begin.tv_sec) + (clock->end.tv_nsec - clock->begin.tv_nsec) * 1.0e-9;
    _output_list.items[_output_list.count - 1].time = time;
    _profiler_clock_stack_remove();
}

void profiler_output(const Profiler_Output_Unit unit)
{
    if (_output_list.count == 0) {
        fprintf(stderr, "%s: no clocks to output\n", _PROFILER_NAME);
        return;
    }
    size_t max_len = 0;
    for (size_t i = 0; i < _output_list.count; ++i) {
        size_t len = strlen(_output_list.items[i].label);
        if (len > max_len) {
            max_len = len;
        }
    }
    for (size_t i = _output_list_count - 1; i >= 0; --i) {
        _profiler_output_list_print(i, max_len);
        free(_output_list.items[i].label);
    }
    _profiler_output_list_free();
}

