#include <cse/entry.hpp>
#include <cse/mono.hpp>
#include <cse/flow.hpp>
#include <cse/console.hpp>
#include <cse/executor.hpp>
#include <cse/ipc.hpp>
#include <cse/executed.h>
#include <functional>
#include <Windows.h>
#include <fstream>

namespace cse
{
    std::string random_string(size_t length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i)
        {
            result += characters[rand() % characters.size()];
        }

        return result;
    }

    nlohmann::json ListResources()
    {
        return nlohmann::json::array();
    }

    nlohmann::json ListResourcesWithRuntimes()
    {
        static auto& executor = Executor::GetInstance();

        nlohmann::json result = nlohmann::json::array();

        for (const auto runtime : executor.GetRuntimes())
        {
            nlohmann::json item = nlohmann::json::object();
            item["resource"] = runtime.GetResourceName();
            result.push_back(item);
        }

        return result;
    }

    nlohmann::json CreateRuntime(const std::string& resource)
    {
        return nlohmann::json::object();
    }

    void ExecuteInResource(const std::string& resource, const std::string& scriptFilePath)
    {
        static auto& executor = Executor::GetInstance();

        std::ifstream scriptFile(scriptFilePath, std::ios::binary);
        if (!scriptFile)
        {
            println("[ExecuteInResource] Failed to open script file: %s", scriptFilePath.c_str());
            return;
        }

        std::vector<uint8_t> scriptData((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
        if (scriptData.empty())
        {
            println("[ExecuteInResource] Script file is empty: %s", scriptFilePath.c_str());
            return;
        }

        for (const cse::RuntimeInfo& runtime : executor.GetRuntimes())
        {
            if (runtime.GetResourceName() == resource)
            {
                auto randomName = random_string(8);
                if (executor.Execute(randomName, scriptData, std::nullopt, std::cref(runtime)))
                {
                    println("[ExecuteInResource] Successfully executed script in resource: %s", resource.c_str());
                }
                else
                {
                    println("[ExecuteInResource] Failed to execute script in resource: %s", resource.c_str());
                }
                return;
            }
        }
    }

    void entrypoint()
    {
        auto deinit = init_console();
        static auto& mono = MonoMethods::GetInstance();
        static auto& executor = Executor::GetInstance();

        static auto& ipc = IpcManager::GetInstance();

        static std::vector<uint8_t> scriptData = std::vector<uint8_t>(
            std::begin(executed), std::end(executed)
        );

        ipc.Initialize([&](const nlohmann::json& request) -> nlohmann::json
        {
            println("[IPC] Received request: %s", request.dump().c_str());

            try
            {
                auto cmd = request.at("cmd").get<std::string>();
                println("[IPC] Command: %s", cmd.c_str());

                nlohmann::json response = nlohmann::json::object();
                if (cmd == "list_resources")
                {
                    response = ListResources();
                }
                else if (cmd == "list_resources_with_runtimes")
                {
                    response = ListResourcesWithRuntimes();
                }
                else if (cmd == "create_runtime")
                {
                    auto resource = request.at("resource").get<std::string>();
                    response = CreateRuntime(resource);
                }
                else if (cmd == "execute_in_resource")
                {
                    auto resource = request.at("resource").get<std::string>();
                    auto scriptFilePath = request.at("scriptFilePath").get<std::string>();
                    ExecuteInResource(resource, scriptFilePath);
                }

                return response;
            }
            catch (std::exception& e)
            {
                println("[IPC] Exception: %s", e.what());
            }

            return nlohmann::json::object();
        });
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