#pragma once

#include <fmt/format.h>
#include <filesystem>
#include <string_view>

/*
 * https://github.com/fmtlib/fmt/issues/2865#issuecomment-1104693640 
 * Make std::filesystem::path formattable.
 * Note: source missing const
 */
template <>
struct fmt::formatter<std::filesystem::path>: formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx) const
    {
        return formatter<std::string_view>::format(path.string(), ctx);
    }
};