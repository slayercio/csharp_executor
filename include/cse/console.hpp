#pragma once
#include <functional>

namespace cse
{
    std::function<void()> init_console();

    void println(const char* fmt, ...);
}