// Ottocento Engine. Architectural BIM Engine.
// Copyright (C) 2024  Lucas M. Faria.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstdarg>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>

//----------------------------------------------------------------------------
/** Output constanTs to color values: **/
#define C_CLEAR  "\033[2J\033[1;1H"
#define C_RESET  "\033[m"
#define C_GREEN  "\033[32m"
#define C_ORANGE "\033[33m"
#define C_YELLOW "\033[93m"
#define C_CYAN   "\033[36m"
#define C_BLUE   "\033[34m"
#define C_RED    "\033[31m"
#define C_WHITE  "\033[37m"
#define C_BOLD   "\033[1m"
#define C_BWHITE "\033[47m"

//----------------------------------------------------------------------------
/** Utils for text layout **/
#define DASHED_SEPARATOR "-------------------------------------"

//----------------------------------------------------------------------------
/** FormaTs the std::string to receive arguments that can be output to the console. **/
inline std::string formatString(const char* format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}

//----------------------------------------------------------------------------
/** Function for formatted logging. **/
enum LogLevel 
{
    normal,
    info,
    debug,
    error,
    warning,
    critical
};

template<LogLevel level = normal, typename... Ts>
constexpr inline void log_t(fmt::format_string<Ts...> fmt, Ts&&... args)
{
    std::string messagePrefix {""};
    auto messageColor { fmt::color::white };

    if constexpr (level == info) {
        messagePrefix = "[info]";
        messageColor = fmt::color::green;
    }
    else if constexpr (level == debug) {
        messagePrefix = "[debug]";
        messageColor = fmt::color::orange;
    }
    else if constexpr (level == warning) {
        messagePrefix = "[warning]";
        messageColor = fmt::color::yellow;
    }
    else if constexpr (level == error) {
        messagePrefix = "[error]";
        messageColor = fmt::color::orange_red;
    }
    else if constexpr (level == critical) {
        messagePrefix = "[critical]";
        messageColor = fmt::color::red;
    }

    fmt::print(fg(messageColor), "{} {}\n", messagePrefix, fmt::format(fmt, std::forward<Ts>(args)...));
}

//----------------------------------------------------------------------------
/** Function to concatenate colored strings. **/
#define COLORED_STRING_WITH_ARGS(color, fmt, ...) (std::string(color) + formatString(fmt, ##__VA_ARGS__) + C_RESET)

//----------------------------------------------------------------------------
/** Colored strings as char* for object naming. Colors can be assigned to each object arbitrarily. **/
#define CSTR_GREEN(fmt, ...)  (std::string(C_GREEN) + formatString(fmt, ##__VA_ARGS__) + C_RESET).c_str()
#define CSTR_YELLOW(fmt, ...) (std::string(C_YELLOW) + formatString(fmt, ##__VA_ARGS__) + C_RESET).c_str()
#define CSTR_RED(fmt, ...)    (std::string(C_RED) + formatString(fmt, ##__VA_ARGS__) + C_RESET).c_str()
#define CSTR_BLUE(fmt, ...)   (std::string(C_BLUE) + formatString(fmt, ##__VA_ARGS__) + C_RESET).c_str()
#define CSTR_CYAN(fmt, ...)   (std::string(C_CYAN) + formatString(fmt, ##__VA_ARGS__) + C_RESET).c_str()
