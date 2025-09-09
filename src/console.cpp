#include <cse/console.hpp>
#include <windows.h>

namespace cse
{
    static bool s_EnableConsole = false;

    auto init_adhesive() -> std::function<void()>
    {
        return []() {};
    }

    auto init_noadhesive() -> std::function<void()>
    {
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONIN$", "r", stdin);
        printf("[CSE] Console initialized\n");
        s_EnableConsole = true;

        return [f]()
        {
            if (f)
            {
                fclose(f);
            }

            FreeConsole();
        };
    }

    // switch between adhesive and no-adhesive version here
    std::function<void()> init_console()
    {
        // return init_noadhesive(); 
    
        return init_adhesive();
    }

    void println(const char* fmt, ...)
    {
        if (!s_EnableConsole)
            return;

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }
}