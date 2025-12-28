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

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>

//----------------------------------------------------------------------------
/** Utils for text layout **/
constexpr char DASHED_SEPARATOR[] { "----------------------------------------------------------------------------" };

//----------------------------------------------------------------------------
/** Log levels for informed logging throughout the engine */
enum LogLevel 
{
    normal,
    info,
    debug,
    error,
    warning,
    critical
};

//----------------------------------------------------------------------------
/** 
 * @brief Wrapper Logger function for the fmt::print that formats and displays information in a standardized
 * way using the LogLevel enum for context information.
 *
 * @code
 * log_t<info>("This is an info logging with param: {}", param);
 * @endcode
 *
 * @param level Log level available from the enum LogLevel. Defaulted to normal.
 */
template<LogLevel level = normal, typename... Ts>
constexpr inline void log_t(fmt::format_string<Ts...> fmt, Ts&&... args)
{
    using enum fmt::color;

    std::string messagePrefix {""};
    auto messageColor { white };

    if constexpr (level == info) {
        messagePrefix = "[info]";
        messageColor = green;
    }
    else if constexpr (level == debug) {
        messagePrefix = "[debug]";
        messageColor = orange;
    }
    else if constexpr (level == warning) {
        messagePrefix = "[warning]";
        messageColor = yellow;
    }
    else if constexpr (level == error) {
        messagePrefix = "[error]";
        messageColor = orange_red;
    }
    else if constexpr (level == critical) {
        messagePrefix = "[critical]";
        messageColor = red;
    }

    fmt::print(fg(messageColor), "{} {}\n", messagePrefix, fmt::format(fmt, std::forward<Ts>(args)...));
}

//----------------------------------------------------------------------------
/** Colored strings as char* for object naming. Colors can be assigned to each object arbitrarily. **/
template<fmt::color color, typename... Ts>
inline std::string color_str(fmt::format_string<Ts...> fmt, Ts&&... args)
{
     return fmt::format(fg(color), fmt, std::forward<Ts>(args)...);
}
