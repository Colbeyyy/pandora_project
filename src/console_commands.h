#pragma once

#include "console.h"

/**
 * @Note about Console Commands
 * 
 * They return true if they should expand or close the console bar
 * They do their own error handling and param parsing
 */
#define CONSOLE_COMMANDS(macro) \
macro(help_command, CH_TEXT("help"), CH_TEXT("")) \
macro(output_log_command, CH_TEXT("log"), CH_TEXT("pushes the parameter to the output log")) \
macro(clear_command, CH_TEXT("clear"), CH_TEXT("clears the console window")) \
macro(toggle_show_logs, CH_TEXT("toggle_show_logs"), CH_TEXT("toggles if we should show logs in the console window")) \
macro(set_show_logs, CH_TEXT("set_show_logs"), CH_TEXT("sets if we should show logs in the console window. Examples \"set_show_logs 0\" \"set_show_logs true\"")) 
