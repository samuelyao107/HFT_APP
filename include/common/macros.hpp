#pragma once

#include <iostream>
#include <cstring>

inline auto ASSERT(bool condition, const std::string &message) noexcept
{
    if (!condition) [[unlikely]]
    {
        std::cerr << "Assertion failed: " << message << std::endl;
        std::abort();
    }
}

inline auto FATAL(const std::string &message) noexcept
{
    std::cerr << "Fatal error: " << message << std::endl;
    std::abort();
}