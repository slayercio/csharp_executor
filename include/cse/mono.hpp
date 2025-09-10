#pragma once
#include <memory>
#include <cse/console.hpp>

namespace cse
{
    using MonoObject = void;
    using MonoClass = void;
    using MonoMethod = void;
    using MonoDomain = void;
    using MonoString = void;
    using MonoImage = void;
    using MonoAssembly = void;
    using MonoType = void;
    using MonoField = void;
    using MonoArray = void;

    class MonoMethods
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;

    public:
        static MonoMethods& GetInstance();

    public:
    // strings
        MonoString* string_new(MonoDomain* domain, const char* str);
        char* string_to_utf8(MonoString* str);

    // memory
        void free(void* ptr);

    // arrays
        MonoArray* array_new(MonoDomain* domain, MonoClass* eclass, uintptr_t n);
        void* array_addr_with_size(void* array, int size, uintptr_t idx);


    // domain and threading
        MonoDomain* get_root_domain();
        MonoDomain* thread_attach(MonoDomain* domain);
        void thread_detach(MonoDomain* thread);
        void domain_foreach(void (*func)(MonoDomain* domain, void* user_data), void* user_data);

        MonoDomain* domain_get();
        void domain_set(MonoDomain* domain);

    // method invocation
        MonoObject* runtime_invoke(MonoMethod* method, MonoObject* obj, void** params, MonoObject** exc);

    // object and class inspection
        MonoClass* object_get_class(MonoObject* obj);
        const char* class_get_name(MonoClass* klass);
        const char* class_get_namespace(MonoClass* klass);
        MonoMethod* class_get_method_from_name(MonoClass* klass, const char* name, int param_count);
        MonoClass* class_from_name(MonoImage* image, const char* name_space, const char* name);
        MonoField* class_get_field_from_name(MonoClass* klass, const char* name);
        void* class_vtable(MonoDomain* domain, MonoClass* klass);

        MonoClass* get_byte_class();
        MonoString* object_to_string(MonoObject* obj, MonoObject** exc);

        void print_exception(MonoObject* exc);

    // field access
        void field_static_get_value(void* vtable, MonoField* field, void** value);
        MonoObject* field_get_value_object(MonoDomain* domain, MonoField* field, MonoObject* obj);

    // compilation
        void* compile_method(MonoMethod* method);

    // assembly and image
        MonoAssembly* domain_open_assembly(MonoDomain* domain, const char* name);
        MonoImage* assembly_get_image(MonoAssembly* assembly);

        const char* domain_get_friendly_name(MonoDomain* domain);

    private:
        MonoMethods();
    };

    struct MonoScope
    {
        MonoDomain* m_OldDomain;
        void* thread;

        inline MonoScope(void* domain)
        {
            static auto& methods = MonoMethods::GetInstance();
            m_OldDomain = methods.domain_get();

            if (!domain)
            {
                println("[CSE] No domain provided!");
                thread = nullptr;
                return;
            }

            thread = methods.thread_attach(domain);
            if (!thread)
            {
                println("[CSE] Failed to attach thread to domain: %p!", domain);
                return;
            }

            methods.domain_set(domain);
        }

        inline MonoScope()
        {
            static auto& methods = MonoMethods::GetInstance();
            m_OldDomain = methods.domain_get();

            MonoDomain* root = methods.get_root_domain();
            if (!root)
            {
                println("[CSE] No root domain found!");
                thread = nullptr;
                return;
            }

            thread = methods.thread_attach(root);
            if (!thread)
            {
                println("[CSE] Failed to attach thread to root domain!");
                return;
            }

            methods.domain_set(root);
        }

        inline ~MonoScope()
        {
            static auto& methods = MonoMethods::GetInstance();
            if (thread)
            {
                methods.thread_detach(thread);
            }

            if (m_OldDomain)
            {
                methods.domain_set(m_OldDomain);
            }
        }
    };
}

#define mono_array_set(array, type, index, value) \
    do { \
        static auto& methods = cse::MonoMethods::GetInstance(); \
        type* __p = (type*) methods.array_addr_with_size(array, sizeof(type), index); \
        *__p = (value); \
    } while (0)