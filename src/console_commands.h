#pragma once

#include "console.h"

#define CONSOLE_COMMANDS(macro) \
macro(help_command, CH_TEXT("help")) \
macro(output_log_command, CH_TEXT("log")) \
macro(clear_command, CH_TEXT("clear"))
