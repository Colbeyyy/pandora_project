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

void log(Log_Severity severity, const tchar* fmt, ...);
void log(const tchar* fmt, ...);
void log_verbose(const tchar* fmt, ...);
void log_warning(const tchar* fmt, ...);
void log_error(const tchar* fmt, ...);