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

            MonoScope scope(domain);
            MonoAssembly* assembly = mono.domain_open_assembly(domain, "CitizenFX.Core");
            if (!assembly)
            {
                println("[CSE] Failed to open CitizenFX.Core assembly in domain\n");
                return;
            }

            MonoImage* image = mono.assembly_get_image(assembly);
            if (!image)
            {
                println("[CSE] Failed to get CitizenFX.Core image\n");
                return;
            }

            MonoClass* klass = mono.class_from_name(image, "CitizenFX.Core", "InternalManager");
            if (!klass)
            {
                println("[CSE] Failed to find InternalManager class\n");
                return;
            }

            MonoField* field = mono.class_get_field_from_name(klass, "<GlobalManager>k__BackingField");
            if (!field)
            {
                println("[CSE] Failed to find GlobalManager field\n");
                return;
            }

            void* vtable = mono.class_vtable(domain, klass);
            if (!vtable)
            {
                println("[CSE] Failed to get vtable for InternalManager\n");
                return;
            }

            void* fieldValue = nullptr;
            mono.field_static_get_value(vtable, field, &fieldValue);
            if (!fieldValue)
            {
                println("[CSE] GlobalManager field is null\n");
                return;
            }

            managers.emplace_back((MonoObject*)fieldValue, domain);
        };

        mono.domain_foreach(callback, &managers);

        return managers;
    }
}