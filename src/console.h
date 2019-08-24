#pragma once

#include <ch_stl/types.h>

void init_console();
void tick_console(f32 dt);
void draw_console();

enum Log_Severity {
	LS_Verbose = 0xFFFFFFFF,
	LS_Warning = 0xFFFF00FF,
	LS_Error   = 0xFF0000FF,
};

void log_full(Log_Severity severity, const tchar* fmt, ...);

#define log(str, ...) log_full(LS_Verbose, str, __VA_ARGS__)
#define log_verbose(str, ...) log_full(LS_Verbose, str, __VA_ARGS__)
#define log_warning(str, ...) log_full(LS_Warning, str, __VA_ARGS__)
#define log_error(str, ...) log_full(LS_Error, str, __VA_ARGS__)