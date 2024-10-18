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

#include <iostream>
#include <cstdarg>

//----------------------------------------------------------------------------
/** Output constants to color values: **/
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
/** Function for formatted logging **/
inline void log(const char* color, const char* level, const char* format, ...)
{
    std::cout << color << "[" << level << "] ";

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    std::cout << C_RESET << std::endl;
}

//----------------------------------------------------------------------------
/** Formats the std::string to receive arguments that can be output to the console. **/
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
/** LOG macros **/
#define LOG_INFO(format, ...)       log(C_GREEN,   "INFO",      format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)      log(C_ORANGE,  "DEBUG",     format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)    log(C_YELLOW,  "WARNING",   format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)      log(C_RED,     "ERROR",     format, ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...)   log(C_RED,     "CRITICAL",  format, ##__VA_ARGS__)

//----------------------------------------------------------------------------
/** Function to concatenate colored strings. **/
#define COLORED_STRING_WITH_ARGS(color, fmt, ...) (std::string(color) + formatString(fmt, ##__VA_ARGS__) + C_RESET)

//----------------------------------------------------------------------------
/** Colored strings as char* for object naming. Colors can be assigned to each object arbitrarily. **/
#define CSTR_GREEN(fmt, ...)  (std::string(C_GREEN) + formatString(fmt, __VA_ARGS__) + C_RESET).c_str()
#define CSTR_YELLOW(fmt, ...) (std::string(C_YELLOW) + formatString(fmt, __VA_ARGS__) + C_RESET).c_str()
#define CSTR_RED(fmt, ...)    (std::string(C_RED) + formatString(fmt, __VA_ARGS__) + C_RESET).c_str()
#define CSTR_BLUE(fmt, ...)   (std::string(C_BLUE) + formatString(fmt, __VA_ARGS__) + C_RESET).c_str()
#define CSTR_CYAN(fmt, ...)   (std::string(C_CYAN) + formatString(fmt, __VA_ARGS__) + C_RESET).c_str()

