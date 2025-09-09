#pragma once
#include <cse/mono.hpp>
#include <string>
#include <vector>
#include <optional>

namespace cse
{
    struct RuntimeInfo
    {
        MonoDomain* m_Domain = nullptr;
        MonoObject* m_InternalManager = nullptr;
        MonoMethod* m_CreateAssemblyInternal = nullptr;

        std::string GetResourceName() const;
    };

    class Executor
    {
    private:
        std::vector<RuntimeInfo> m_Runtimes;

    public:
        static Executor& GetInstance();

        /**
         * @brief Executes a C# script within the specified runtime environment.
         * @param scriptName The name of the script to be executed.
         * @param scriptData The bytecode of the C# script.
         * @param pdbData Optional PDB data for debugging purposes.
         * @param runtime Optional runtime environment to use. If not provided, the first available runtime will be used.
         * @return true if the script was executed successfully, false otherwise.
         */
        bool Execute(const std::string& scriptName, const std::vector<uint8_t> &scriptData, 
            std::optional<std::reference_wrapper<std::vector<uint8_t>>> pdbData = std::nullopt, std::optional<std::reference_wrapper<RuntimeInfo>> runtime = std::nullopt);

    private:
        /**
         * @brief Finds and initializes all available Mono runtimes in the current process.
         * This method populates the m_Runtimes vector with information about each detected runtime.
         */
        void FindRuntimes();
    };
}