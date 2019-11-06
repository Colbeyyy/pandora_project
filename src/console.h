#pragma once

#include "ch_stl/string.h"

void init_console();
void tick_console(f32 dt);
void draw_console();

enum Log_Severity {
	LS_Verbose = 0xFFFFFFFF,
	LS_Warning = 0xFFFF00FF,
	LS_Error   = 0xFF0000FF,
};

/**
 * Used for outputting debug info to the console
 * Has severity and time stamp
 * Severity is used for color
 */
void output_log(Log_Severity severity, const char* fmt, ...);
/**
 * Used by commands for outputting a log into the console for help info etc
 * Has time stamp 
 */
void console_log(const char* fmt, ...);

#define o_log(str, ...) output_log(LS_Verbose, str, __VA_ARGS__)
#define o_log_verbose(str, ...) output_log(LS_Verbose, str, __VA_ARGS__)
#define o_log_warning(str, ...) output_log(LS_Warning, str, __VA_ARGS__)
#define o_log_error(str, ...) output_log(LS_Error, str, __VA_ARGS__)

void handle_uneeded_params(const char* command);

bool help_command(const ch::String& params);
bool output_log_command(const ch::String& params);
bool clear_command(const ch::String& params);
bool toggle_show_logs(const ch::String& params);
bool set_show_logs(const ch::String& params);
bool reset(const ch::String& params);