#include <cse/ipc.hpp>
#include <cse/console.hpp>
#include <windows.h>
#include <optional>

namespace cse
{
    struct IpcManager::Impl
    {
        IpcCallback m_Callback;
        std::thread m_ListenerThread;
        std::atomic_bool m_Running;

        HANDLE m_PipeHandle;

    private:
        bool ReadExact(HANDLE pipe, void* buffer, size_t size)
        {
            char* buf = static_cast<char*>(buffer);
            size_t totalRead = 0;

            while (totalRead < size)
            {
                DWORD read = 0;
                if (!ReadFile(pipe, buf + totalRead, static_cast<DWORD>(size - totalRead), &read, NULL) || read == 0)
                {
                    println("Failed to read from pipe. Error: %x", GetLastError());

                    return false;
                }

                if (read == 0)
                {
                    println("Pipe closed by client.");

                    return false;
                }

                totalRead += read;
            }

            return true;
        }

        bool WriteAll(HANDLE pipe, const void* buffer, size_t size)
        {
            const char* buf = static_cast<const char*>(buffer);
            size_t totalWritten = 0;

            while (totalWritten < size)
            {
                DWORD written = 0;
                if (!WriteFile(pipe, buf + totalWritten, static_cast<DWORD>(size - totalWritten), &written, NULL) || written == 0)
                {
                    println("Failed to write to pipe. Error: %x", GetLastError());

                    return false;
                }

                totalWritten += written;
            }

            return true;
        }

        std::optional<nlohmann::json> ReceiveJson(HANDLE pipe)
        {
            uint32_t length = 0;
            if (!ReadExact(pipe, &length, sizeof(length)))
            {
                return std::nullopt;
            }

            if (length == 0)
            {
                return nlohmann::json::object();
            }

            std::vector<char> buffer(length);
            if (!ReadExact(pipe, buffer.data(), length))
            {
                return std::nullopt;
            }

            try
            {
                return nlohmann::json::parse(buffer);
            }
            catch(const std::exception& e)
            {
                println("Failed to parse JSON: %s", e.what());
                return std::nullopt;
            }
        }

        bool SendJson(HANDLE pipe, const nlohmann::json& json)
        {
            std::string serialized = json.dump();
            uint32_t length = static_cast<uint32_t>(serialized.size());

            if (!WriteAll(pipe, &length, sizeof(length)))
            {
                return false;
            }

            if (!WriteAll(pipe, serialized.data(), length))
            {
                return false;
            }

            return true;
        }

        void ClientThread(HANDLE pipe)
        {
            while (true)
            {
                auto request = ReceiveJson(pipe);
                if (!request.has_value())
                {
                    break;
                }

                auto response = m_Callback(*request);
                if (response.is_null())
                {
                    break;
                }   

                if (!SendJson(pipe, response))
                {
                    break;
                }
            }

            FlushFileBuffers(pipe);
            DisconnectNamedPipe(pipe);
            CloseHandle(pipe);
        }

    public:
        void Initialize(const wchar_t* pipeName)
        {
            m_Running = true;

            m_ListenerThread = std::thread([&, pipeName]()
            {
                while (m_Running)
                {
                    this->m_PipeHandle = CreateNamedPipeW(
                        pipeName,
                        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                        PIPE_UNLIMITED_INSTANCES,
                        64 * 1024,
                        64 * 1024,
                        NULL,
                        NULL
                    );

                    if (m_PipeHandle == INVALID_HANDLE_VALUE)
                    {
                        println("Failed to create named pipe. Error: %x", GetLastError());
                        Sleep(1000);

                        continue;
                    }

                    println("Waiting for client to connect to pipe...");
                    BOOL ok = ConnectNamedPipe(m_PipeHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
                    if (!ok)
                    {
                        println("Failed to connect to named pipe. Error: %x", GetLastError());
                        CloseHandle(m_PipeHandle);
                        Sleep(1000);

                        continue;
                    }

                    println("Named pipe created successfully.");
                    std::thread (&Impl::ClientThread, this, m_PipeHandle).detach();
                }
            });
        }

        void Shutdown()
        {
            m_Running = false;
            if (m_ListenerThread.joinable())
            {
                m_ListenerThread.join();
            }
        }
    };

    IpcManager& IpcManager::GetInstance()
    {
        static IpcManager instance;
        return instance;
    }

    void IpcManager::Initialize(IpcCallback callback, const wchar_t* pipeName)
    {
        m_Impl = std::make_unique<Impl>();
        m_Impl->m_Callback = std::move(callback);
        m_Impl->Initialize(pipeName);
    }

    void IpcManager::Shutdown()
    {
        m_Impl->Shutdown();
    }
}