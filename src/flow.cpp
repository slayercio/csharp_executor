#include <cse/flow.hpp>
#include <cse/console.hpp>

namespace cse
{
    std::vector<std::pair<MonoObject*, MonoDomain*>> FindAllInternalManagers()
    {
        static auto& mono = MonoMethods::GetInstance();
        std::vector<std::pair<MonoObject*, MonoDomain*>> managers;

        auto callback = [](MonoDomain* domain, void* user_data)
        {
            static auto& mono = MonoMethods::GetInstance();
            auto& managers = *reinterpret_cast<std::vector<std::pair<MonoObject*, MonoDomain*>>*>(user_data);

            auto thread = mono.thread_attach(domain);
            if (!thread)
            {
                println("[CSE] Failed to attach to domain: %p\n", domain);
                return;
            }

            mono.domain_set(domain);

            auto cleanup = [&]()
            {
                mono.domain_set(mono.get_root_domain());
                mono.thread_detach(thread);
            };

            MonoAssembly* assembly = mono.domain_open_assembly(domain, "CitizenFX.Core");
            if (!assembly)
            {
                println("[CSE] Failed to open CitizenFX.Core assembly in domain\n");
                cleanup();
                return;
            }

            MonoImage* image = mono.assembly_get_image(assembly);
            if (!image)
            {
                println("[CSE] Failed to get CitizenFX.Core image\n");
                cleanup();
                return;
            }

            MonoClass* klass = mono.class_from_name(image, "CitizenFX.Core", "InternalManager");
            if (!klass)
            {
                println("[CSE] Failed to find InternalManager class\n");
                cleanup();
                return;
            }

            MonoField* field = mono.class_get_field_from_name(klass, "<GlobalManager>k__BackingField");
            if (!field)
            {
                println("[CSE] Failed to find GlobalManager field\n");
                cleanup();
                return;
            }

            void* vtable = mono.class_vtable(domain, klass);
            if (!vtable)
            {
                println("[CSE] Failed to get vtable for InternalManager\n");
                cleanup();
                return;
            }

            void* fieldValue = nullptr;
            mono.field_static_get_value(vtable, field, &fieldValue);
            if (!fieldValue)
            {
                println("[CSE] GlobalManager field is null\n");
                cleanup();
                return;
            }

            managers.push_back(std::make_pair((MonoObject*)fieldValue, domain));
            cleanup();
        };

        mono.domain_foreach(callback, &managers);

        return managers;
    }
}