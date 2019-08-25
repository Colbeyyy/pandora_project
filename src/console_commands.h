#pragma once

#include "console.h"

/**
 * @Note about Console Commands
 * 
 * They return true if they should expand or close the console bar
 * They do their own error handling and param parsing
 */
#define CONSOLE_COMMANDS(macro) \
macro(help_command, CH_TEXT("help")) \
macro(output_log_command, CH_TEXT("log")) \
macro(clear_command, CH_TEXT("clear")) \
macro(toggle_show_logs, CH_TEXT("toggle_show_logs")) \
macro(set_show_logs, CH_TEXT("set_show_logs")) 
