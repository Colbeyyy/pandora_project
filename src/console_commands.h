#pragma once

#include "console.h"

bool toggle_fps(const ch::String& params);

/**
 * @Note about Console Commands
 * 
 * They return true if they should expand or close the console bar
 * They do their own error handling and param parsing
 */
#define CONSOLE_COMMANDS(macro) \
macro(help_command, "help", "") \
macro(output_log_command, "log", "pushes the parameter to the output log") \
macro(clear_command, "clear", "clears the console window") \
macro(toggle_show_logs, "toggle_show_logs", "toggles if we should show logs in the console window") \
macro(set_show_logs, "set_show_logs", "sets if we should show logs in the console window. Examples \"set_show_logs 0\" \"set_show_logs true\"") // \
// macro(toggle_fps, "toggle_fps", "toggles the fps counter")
