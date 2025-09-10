#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <nlohmann/json.hpp>

namespace cse
{
    inline auto IPC_PIPE_NAME = L"\\\\.\\pipe\\my_ipc_pipe";

    // request -> response
    using IpcCallback = std::function<nlohmann::json(const nlohmann::json&)>;

    class IpcManager
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    public:
        static IpcManager& GetInstance();

    public:
        void Initialize(IpcCallback callback, const wchar_t* pipeName = IPC_PIPE_NAME);
        void Shutdown();
    };
}