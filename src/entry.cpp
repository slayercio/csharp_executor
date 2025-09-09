#include <cse/entry.hpp>
#include <cse/mono.hpp>
#include <cse/flow.hpp>
#include <cse/console.hpp>
#include <cse/executor.hpp>
#include <cse/executed.h>
#include <functional>
#include <Windows.h>

namespace cse
{
    void entrypoint()
    {
        auto deinit = init_console();
        static auto& mono = MonoMethods::GetInstance();
        static auto& executor = Executor::GetInstance();

        static std::vector<uint8_t> scriptData = std::vector<uint8_t>(
            std::begin(executed), std::end(executed)
        );

        while (!GetAsyncKeyState(VK_END))
        {
            if (GetAsyncKeyState(VK_HOME))
            {
                executor.Execute("test_script", scriptData);

                Sleep(500);
            }

            Sleep(100);
        }

        deinit();
    }
}