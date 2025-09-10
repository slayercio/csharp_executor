#include <cse/executor.hpp>
#include <cse/flow.hpp>

namespace cse
{
    std::string RuntimeInfo::GetResourceName() const
    {
        if (!m_InternalManager || !m_CreateAssemblyInternal || !m_Domain)
            return {};

        std::string name;

        {
            MonoScope scope(m_Domain);
            static MonoMethods& methods = MonoMethods::GetInstance();

            static MonoClass* internalManagerClass = nullptr;
            if (!internalManagerClass)
            {
                internalManagerClass = methods.object_get_class(m_InternalManager);
                if (!internalManagerClass)
                {
                    println("[CSE] Failed to get InternalManager class!");
                    return {};
                }
            }

            static MonoField* nameField = nullptr;
            if (!nameField)
            {
                nameField = methods.class_get_field_from_name(internalManagerClass, "m_resourceName");
                if (!nameField)
                {
                    println("[CSE] Failed to get InternalManager.m_resourceName field!");
                    return {};
                }
            }

            MonoObject* nameObj = methods.field_get_value_object(m_Domain, nameField, m_InternalManager);
            if (!nameObj)
            {
                println("[CSE] Failed to get InternalManager.m_resourceName value!");
                return {};
            }

            char* nameStr = methods.string_to_utf8((MonoString*)nameObj);
            if (!nameStr)
            {
                println("[CSE] Failed to convert InternalManager.m_resourceName to UTF-8!");
                return {};
            }

            name = std::string(nameStr);
            methods.free(nameStr);
        }

        return name;
    }

    Executor& Executor::GetInstance()
    {
        static Executor instance;
        return instance;
    }

    bool Executor::Execute(const std::string& scriptName, const std::vector<uint8_t>& scriptData,
        std::optional<std::reference_wrapper<std::vector<uint8_t>>> pdbData,
        std::optional<std::reference_wrapper<const RuntimeInfo>> runtime)
    {
        static MonoMethods& methods = MonoMethods::GetInstance();

        RuntimeInfo info;
        if (runtime.has_value())
        {
            info = runtime->get();
        }
        else
        {
            FindRuntimes();

            if (m_Runtimes.empty())
            {
                println("[CSE] No runtimes available to execute script!");
                return false;
            }

            info = m_Runtimes[0];
        }

        if (!info.m_Domain || !info.m_InternalManager || !info.m_CreateAssemblyInternal)
        {
            println("[CSE] Invalid runtime info provided!");
            return false;
        }

        auto resourceName = info.GetResourceName();
        auto domainName = methods.domain_get_friendly_name(info.m_Domain);
        println("[CSE] Executing a script in domain: %s, resource: %s", domainName, resourceName.c_str());

        {
            MonoScope scope(info.m_Domain);

            MonoString* name = methods.string_new(info.m_Domain, scriptName.c_str());
            if (!name)
            {
                println("[CSE] Failed to create MonoString for script name!");
                return false;
            }

            MonoClass* byteClass = methods.get_byte_class();
            if (!byteClass)
            {
                println("[CSE] Failed to get Mono byte class!");
                return false;
            }

            MonoArray* scriptArray = (MonoArray*)methods.array_new(info.m_Domain, byteClass, scriptData.size());
            if (!scriptArray)
            {
                println("[CSE] Failed to create MonoArray for script data!");
                return false;
            }

            for (size_t i = 0; i < scriptData.size(); ++i)
            {
                mono_array_set(scriptArray, uint8_t, i, scriptData[i]);
            }

            MonoArray* pdbArray = nullptr;
            if (pdbData.has_value())
            {
                pdbArray = (MonoArray*)methods.array_new(info.m_Domain, byteClass, pdbData->get().size());
                if (!pdbArray)
                {
                    println("[CSE] Failed to create MonoArray for PDB data!");
                    return false;
                }

                for (size_t i = 0; i < pdbData->get().size(); ++i)
                {
                    mono_array_set(pdbArray, uint8_t, i, pdbData->get()[i]);
                }
            }
            else
            {
                pdbArray = (MonoArray*)methods.array_new(info.m_Domain, byteClass, 0);
                if (!pdbArray)
                {
                    println("[CSE] Failed to create empty MonoArray for PDB data!");
                    return false;
                }
            }

            MonoObject* exc = nullptr;
            void* args[] = { name, scriptArray, pdbArray };
            methods.runtime_invoke(info.m_CreateAssemblyInternal, info.m_InternalManager, args, &exc);
            if (exc)
            {
                println("[CSE] Exception occurred while executing script!");
                methods.print_exception(exc);
                return false;
            }

            println("[CSE] Script executed successfully!");
            return true;
        }
    }

    const std::vector<RuntimeInfo>& Executor::GetRuntimes()
    {
        FindRuntimes();
        
        std::vector<RuntimeInfo> validRuntimes;
        for (const auto& runtime : m_Runtimes)
        {
            auto it = std::find_if(validRuntimes.begin(), validRuntimes.end(), [&](const RuntimeInfo& r)
            {
                return r.GetResourceName() == runtime.GetResourceName();
            });

            if (it != validRuntimes.end())
            {
                continue;
            }

            if (runtime.m_Domain && runtime.m_InternalManager && runtime.m_CreateAssemblyInternal && runtime.GetResourceName().length() > 0)
            {
                println("[CSE] Found valid runtime in resource: %s", runtime.GetResourceName().c_str());
                validRuntimes.push_back(runtime);
            }
        }

        m_Runtimes = std::move(validRuntimes);
        return m_Runtimes;
    }

    void Executor::FindRuntimes()
    {
        auto managers = FindAllInternalManagers();

        for (const auto& [manager, domain] : managers)
        {
            RuntimeInfo info;
            info.m_Domain = domain;
            info.m_InternalManager = manager;

            static MonoMethods& methods = MonoMethods::GetInstance();
            {
                MonoScope scope(domain);

                MonoClass* internalManagerClass = methods.object_get_class(manager);
                if (!internalManagerClass)
                {
                    println("[CSE] Failed to get InternalManager class!");
                    continue;
                }

                MonoMethod* createAssemblyMethod = methods.class_get_method_from_name(internalManagerClass, "CreateAssemblyInternal", 3);
                if (!createAssemblyMethod)
                {
                    println("[CSE] Failed to get InternalManager.CreateAssemblyInternal method!");
                    continue;
                }

                info.m_CreateAssemblyInternal = createAssemblyMethod;
            }

            m_Runtimes.push_back(info);
        }
    }
}