#ifndef CBC__H
#define CBC__H

#ifndef CBCDEF
    #define CBCDEF
#endif // CBCDEF

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

typedef enum {
	CBC_DEBUG,
	CBC_INFO,
	CBC_WARNING,
	CBC_ERROR,
	CBC_NONE
} Cbc_Log_Level;

extern const char* cbc__log_level_names[];

extern Cbc_Log_Level cbc__log_level;

CBCDEF void cbc_change_log_level(Cbc_Log_Level log_level);
CBCDEF FILE* cbc_log_get_output_stream(Cbc_Log_Level log_level);
CBCDEF void cbc_log_empty_line(Cbc_Log_Level log_level);
CBCDEF void cbc_log(Cbc_Log_Level log_level, const char* format, ...);

CBCDEF char* cbc_format_string(const char* format, ...);
CBCDEF void cbc_alloc_check(void* ptr);

#define CBC__CMD_CAPACITY_INCREMENT 16

typedef struct {
	const char** items;
	size_t count;
	size_t capacity;
} Cbc_Cmd;

CBCDEF void cbc__cmd_append(Cbc_Cmd* cmd, ...);
#define cbc_cmd_append(cmd, ...) \
	cbc__cmd_append((cmd), __VA_ARGS__, NULL)

#define cbc_cc(cmd) cbc_cmd_append(cmd, "cc")
#define cbc_cc_std(cmd, version) cbc_cmd_append(cmd, cbc_format_string("-std=%s", (version)))
#define cbc_cc_flags(cmd, ...) cbc_cmd_append(cmd, "-Wall", "-Wextra", ##__VA_ARGS__)
#define cbc_cc_output(cmd, output) cbc_cmd_append(cmd, "-o", (output))
#define cbc_cc_inputs(cmd, ...) cbc_cmd_append(cmd, __VA_ARGS__)

CBCDEF void cbc_cmd_reset(Cbc_Cmd* cmd);
CBCDEF void cbc_log_cmd(Cbc_Cmd* cmd);
CBCDEF char* cbc_cmd_construct(Cbc_Cmd* cmd);
CBCDEF int cbc_cmd_run(Cbc_Cmd*);

CBCDEF bool cbc_rename_file(const char* old, const char* new);
CBCDEF bool cbc_delete_file(const char* path);

#define cbc__rebuild(bin, src) "cc", "-o", bin, src
#define cbc_auto_rebuild(argc, argv) \
	cbc__auto_rebuild(argc, argv, __FILE__)
CBCDEF void cbc__auto_rebuild(int argc, char** argv, const char* src);

CBCDEF int cbc__needs_rebuild(const char* output, const char* input);

#endif // CBC__H

#ifdef CBC_IMPLEMENTATION

const char* cbc__log_level_names[] = {
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"NONE"
};

Cbc_Log_Level cbc__log_level = CBC_INFO;

CBCDEF void cbc_change_log_level(Cbc_Log_Level log_level)
{
	cbc__log_level = log_level;
	cbc_log(CBC_INFO, "changed log level to %s", cbc__log_level_names[cbc__log_level]);
}

CBCDEF FILE* cbc_log_get_output_stream(Cbc_Log_Level log_level)
{
	switch (log_level) {
		case CBC_DEBUG:
		case CBC_INFO:
			return stdout;
		case CBC_WARNING:
		case CBC_ERROR:
			return stderr;
		default:
			return stderr;
	}
}

CBCDEF void cbc_log_empty_line(Cbc_Log_Level log_level)
{
	if (log_level >= cbc__log_level) {
		FILE* out = cbc_log_get_output_stream(log_level);
		fprintf(out, "\n");
	}
}

CBCDEF void cbc_log(Cbc_Log_Level log_level, const char* format, ...)
{
	if (log_level >= cbc__log_level) {
		va_list args;
		va_start(args, format);
		FILE* out = cbc_log_get_output_stream(log_level);
		switch (log_level) {
			case CBC_DEBUG:
				fprintf(out, "[DEBUG]   ");
				break;
			case CBC_INFO:
				fprintf(out, "[INFO]    ");
				break;
			case CBC_WARNING:
				fprintf(out, "[WARNING] ");
				break;
			case CBC_ERROR:
				fprintf(out, "[ERROR]   ");
				break;
			default:
				va_end(args);
				return;
		}
		vfprintf(out, format, args);
		fprintf(out, "\n");
		va_end(args);
	}
}

CBCDEF char* cbc_format_string(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t len = vsnprintf(NULL, 0, format, args);
	va_end(args);
	if (len < 0) {
		return strdup("");
	}
	char* str = (char*)malloc(sizeof(char) * (len + 1));
	cbc_alloc_check(str);
	va_start(args, format);
	vsnprintf(str, len + 1, format, args);
	va_end(args);
	return str;
}

CBCDEF void cbc_alloc_check(void* ptr)
{
	if (!ptr) {
		cbc_log(CBC_ERROR, "allocation error");
		exit(EXIT_FAILURE);
	}
}

CBCDEF void cbc__cmd_append(Cbc_Cmd* cmd, ...)
{
	va_list args;
	va_start(args, cmd);
	const char* item;
	while ((item = va_arg(args, const char*)) != NULL) {
		if (cmd->count >= cmd->capacity) {
			cmd->capacity += CBC__CMD_CAPACITY_INCREMENT;
			cmd->items = realloc(cmd->items, cmd->capacity * sizeof(*cmd->items));
			cbc_log(CBC_DEBUG, "extended cmd: %p capacity to %zu", cmd, cmd->capacity);
			cbc_alloc_check(cmd->items);
		}
		cmd->items[cmd->count++] = item;
		cbc_log(CBC_DEBUG, "added item: \"%s\" to cmd: %p", cmd->items[cmd->count - 1], cmd);
	}
	va_end(args);
}

CBCDEF void cbc_cmd_reset(Cbc_Cmd* cmd)
{
	free(cmd->items);
	cmd->items = NULL;
	cmd->count = 0;
	cmd->capacity = 0;
	cbc_log(CBC_DEBUG, "reset the cmd: %p", cmd);
}

CBCDEF void cbc_log_cmd(Cbc_Cmd* cmd)
{
	cbc_log(CBC_INFO, "cmd = %p", cmd);
	cbc_log(CBC_INFO, "cmd->capacity = %zu", cmd->capacity);
	cbc_log(CBC_INFO, "cmd->count = %zu", cmd->count);
	cbc_log(CBC_INFO, "cmd->items = %p", cmd->items);
	for (size_t i = 0; i < cmd->count; ++i) {
		cbc_log(CBC_INFO, "    cmd->items[%zu] = \"%s\"", i, cmd->items[i]);
	}
	cbc_log_empty_line(CBC_INFO);
}

CBCDEF char* cbc_cmd_construct(Cbc_Cmd* cmd)
{
	if (cmd->count == 0) {
		cbc_log(CBC_WARNING, "attempted to construct command from 0 items");
		return strdup("");
	}
	size_t size = 0;
	for (size_t i = 0; i < cmd->count; ++i) {
		size += strlen(cmd->items[i]);
	}
	size += cmd->count - 1;
	cbc_log(CBC_DEBUG, "length of command from cmd: %p is %zu", cmd, size);
	char* command = (char*)malloc(sizeof(char) * (size + 1));
	cbc_alloc_check(command);
	size_t offset = 0;
	for (size_t i = 0; i < cmd->count; ++i) {
		strcpy(command + offset, cmd->items[i]);
		offset += strlen(cmd->items[i]);
		command[offset++] = ' ';
	}
	command[size] = '\0';
	cbc_log(CBC_DEBUG, "constructed command: \"%s\" from cmd: %p", command, cmd);
	return command;
}

CBCDEF int cbc_cmd_run(Cbc_Cmd* cmd)
{
	char* command = cbc_cmd_construct(cmd);
	cbc_cmd_reset(cmd);
	cbc_log(CBC_DEBUG, "executing: \"%s\"", command);
	int status = system(command);
	if (status == -1) {
		cbc_log(CBC_ERROR, "failed to execute command: \"%s\"", command);
		free(command);
		return -1;
	}
	if (WIFEXITED(status)) {
		int exit_code = WEXITSTATUS(status);
		if (exit_code == 0) {
			cbc_log(CBC_DEBUG, "command: \"%s\" executed succesfully", command);
		}
		else {
			cbc_log(CBC_ERROR, "command: \"%s\" failed with exit code: %d", command, exit_code);
		}
		free(command);
		return exit_code;
	}
	cbc_log(CBC_ERROR, "command: \"%s\" did not terminate normally", command);
	free(command);
	return -2;
}

CBCDEF bool cbc_rename_file(const char* old, const char* new)
{
	cbc_log(CBC_DEBUG, "renaming %s -> %s", old, new);
	if (rename(old, new) < 0) {
		cbc_log(CBC_ERROR, "could not rename %s to %s", old, new);
		return false;
	}
	return true;
}

CBCDEF bool cbc_delete_file(const char* path)
{
	cbc_log(CBC_DEBUG, "deleting %s", path);
	if (remove(path) < 0) {
		cbc_log(CBC_ERROR, "could not delete %s", path);
		return false;
	}
	return  true;
}

CBCDEF void cbc__auto_rebuild(int argc, char** argv, const char* src)
{
	const char* bin = argv[0];
	if (strncmp(bin, "./", 2) == 0) {
		bin += 2;
	}
	int rebuild_is_needed = cbc__needs_rebuild(bin, src);
	if (rebuild_is_needed < 0) {
		cbc_log(CBC_ERROR, "auto rebuild error");
		exit(EXIT_FAILURE);
	}
	if (!rebuild_is_needed) {
		return;
	}
	cbc_log(CBC_DEBUG, "rebuilding cbuiltc");
	Cbc_Cmd cmd = {0};
	const char* old_bin = cbc_format_string("%s.old", bin);
	if (!cbc_rename_file(bin, old_bin)) {
		cbc_log(CBC_ERROR, "failed to rebuild %s", src);
		exit(EXIT_FAILURE);
	}
	cbc_cmd_append(&cmd, cbc__rebuild(bin, src));
	if (cbc_cmd_run(&cmd)) {
		cbc_rename_file(old_bin, bin);
		cbc_log(CBC_ERROR, "failed to rebuild %s", src);
		exit(EXIT_FAILURE);
	}
	cbc_delete_file(old_bin);
	cbc_cmd_append(&cmd, cbc_format_string("./%s", bin));
	if (cbc_cmd_run(&cmd)) {
        cbc_log(CBC_ERROR, "failed to rebuild %s", src);
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

CBCDEF int cbc__needs_rebuild(const char* output, const char* input)
{
	struct stat stat_buffer = {0};
	if (stat(output, &stat_buffer) < 0) {
		if (errno == ENOENT) {
			return 1;
		}
		cbc_log(CBC_ERROR, "could not stat %s", output);
		return -1;
	}
	int output_time = stat_buffer.st_mtime;
	if (stat(input, &stat_buffer) < 0) {
		cbc_log(CBC_ERROR, "could not stat %s", input);
		return -1;
	}
	int input_time = stat_buffer.st_mtime;
	if (input_time > output_time) {
		return 1;
	}
	return 0;
}

#endif // CBC_IMPLEMENTATION

#ifndef CBC__STRIP_PREFIX_GUARD
#define CBC__STRIP_PREFIX_GUARD
	#ifdef CBC_STRIP_PREFIX
		#define Log_Level Cbc_Log_Level
		#define DEBUG CBC_DEBUG
		#define INFO CBC_INFO
		#define WARNING CBC_WARNING
		#define ERROR CBC_ERROR
		#define NONE CBC_NONE
		#define change_log_level cbc_change_log_level
		#define log_get_output_stream cbc_log_get_output_stream
		#define log_empty_line cbc_log_empty_line
		#define log cbc_log
		#define format_string cbc_format_string
		#define alloc_check cbc_alloc_check
		#define Cmd Cbc_Cmd
		#define cmd_append cbc_cmd_append
		#define cc cbc_cc
        #define cc_std cbc_cc_std
		#define cc_flags cbc_cc_flags
		#define cc_output cbc_cc_output
		#define cc_inputs cbc_cc_inputs
		#define cmd_reset cbc_cmd_reset
		#define log_cmd cbc_log_cmd
		#define cmd_construct cbc_cmd_construct
		#define cmd_run cbc_cmd_run
		#define rename_file cbc_rename_file
		#define delete_file cbc_delete_file
		#define auto_rebuild cbc_auto_rebuild
	#endif // CBC_STRIP_PREFIX
#endif // CBC__STRIP_PREFIX_GUARD

