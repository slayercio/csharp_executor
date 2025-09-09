#include <cse/mono.hpp>
#include <windows.h>
#include <stdexcept>
#include <format>

namespace cse
{
    namespace typedefs
    {
        /**
         * Mono function typedefs
         * These are function pointer types corresponding to Mono's C API functions.
        */

        // New string from C string
        using string_new_func = MonoString* (*)(MonoDomain* domain, const char* str);

        // Mono free
        using free_func = void (*)(void* ptr);

        // MonoString to UTF-8 C string
        using string_to_utf8_func = char* (*)(MonoString* str);

        // JIT compile method MonoMethod* -> compiled code pointer
        using compile_method_func = void* (*)(MonoMethod* method);

        // Get the root domain
        using get_root_domain_func = MonoDomain* (*)();

        // Attach the current thread to a domain
        using thread_attach_func = MonoDomain* (*)(MonoDomain* domain);

        // Detach the current thread from a domain
        using thread_detach_func = void (*)(MonoDomain* thread);

        // Invoke a method
        using runtime_invoke_func = MonoObject* (*)(MonoMethod* method, MonoObject* obj, void** params, MonoObject** exc);

        // Get object's class
        using object_get_class_func = MonoClass* (*)(MonoObject* obj);

        // Get class name
        using class_get_name_func = const char* (*)(MonoClass* klass);

        // Get class namespace
        using class_get_namespace_func = const char* (*)(MonoClass* klass);

        // Get class method from name
        using class_get_method_from_name_func = MonoMethod* (*)(MonoClass* klass, const char* name, int param_count);

        // Domain iteration
        using domain_foreach_func = void (*)(void (*func)(MonoDomain* domain, void* user_data), void* user_data);
    
        // Get/set current domain
        using domain_get_func = MonoDomain* (*)();
        using domain_set_func = void (*)(MonoDomain* domain);

        // Class from name in image
        using class_from_name_func = MonoClass* (*)(MonoImage* image, const char* name_space, const char* name);
        
        // Open assembly in domain
        using domain_open_assembly_func = MonoAssembly* (*)(MonoDomain* domain, const char* name);
        
        // Get image from assembly
        using assembly_get_image_func = MonoImage* (*)(MonoAssembly* assembly);

        // Get field from class by name
        using class_get_field_from_name_func = MonoField* (*)(MonoClass* klass, const char* name);

        // Get vtable for class in domain
        using class_vtable_func = void* (*)(MonoDomain* domain, MonoClass* klass);

        // Static field get value
        using field_static_get_value_func = void (*)(void* vtable, MonoField* field, void** value);
        using field_get_value_object_func = MonoObject* (*)(MonoDomain* domain, MonoField* field, MonoObject* obj);

        using domain_get_friendly_name_func = const char* (*)(MonoDomain* domain);

        using array_new_func = void* (*)(MonoDomain* domain, MonoClass* eclass, uintptr_t n);
        using array_addr_with_size_func = void* (*)(void* array, int size, uintptr_t idx);

        using get_byte_class_func = MonoClass* (*)();

        using object_to_string_func = MonoString* (*)(MonoObject* obj, MonoObject** exc);
    }

    struct MonoMethods::Impl
    {
    public:
        // Function pointers to Mono C API functions
        typedefs::string_new_func string_new = nullptr;
        typedefs::string_to_utf8_func string_to_utf8 = nullptr;
        typedefs::compile_method_func compile_method = nullptr;
        typedefs::free_func free = nullptr;
        typedefs::get_root_domain_func get_root_domain = nullptr;
        typedefs::thread_attach_func thread_attach = nullptr;
        typedefs::thread_detach_func thread_detach = nullptr;
        typedefs::runtime_invoke_func runtime_invoke = nullptr;
        typedefs::object_get_class_func object_get_class = nullptr;
        typedefs::class_get_name_func class_get_name = nullptr;
        typedefs::class_get_namespace_func class_get_namespace = nullptr;
        typedefs::class_get_method_from_name_func class_get_method_from_name = nullptr;
        typedefs::domain_foreach_func domain_foreach = nullptr;
        typedefs::domain_get_func domain_get = nullptr;
        typedefs::domain_set_func domain_set = nullptr;
        typedefs::class_from_name_func class_from_name = nullptr;
        typedefs::domain_open_assembly_func domain_open_assembly = nullptr;
        typedefs::assembly_get_image_func assembly_get_image = nullptr;
        typedefs::class_get_field_from_name_func class_get_field_from_name = nullptr;
        typedefs::class_vtable_func class_vtable = nullptr;
        typedefs::field_static_get_value_func field_static_get_value = nullptr;
        typedefs::field_get_value_object_func field_get_value_object = nullptr;
        typedefs::domain_get_friendly_name_func domain_get_friendly_name = nullptr;
        typedefs::array_new_func array_new = nullptr;
        typedefs::array_addr_with_size_func array_addr_with_size = nullptr;
        typedefs::get_byte_class_func get_byte_class = nullptr;
        typedefs::object_to_string_func object_to_string = nullptr;

    public:
        Impl()
        {
            HMODULE hModule = GetModuleHandleA("mono-2.0-sgen.dll");
            if (!hModule)
            {
                throw std::runtime_error("Mono module not found");
            }

            string_new = (typedefs::string_new_func)GetProcAddress(hModule, "mono_string_new");
            string_to_utf8 = (typedefs::string_to_utf8_func)GetProcAddress(hModule, "mono_string_to_utf8");
            free = (typedefs::free_func)GetProcAddress(hModule, "mono_free");
            get_root_domain = (typedefs::get_root_domain_func)GetProcAddress(hModule, "mono_get_root_domain");
            thread_attach = (typedefs::thread_attach_func)GetProcAddress(hModule, "mono_thread_attach");
            thread_detach = (typedefs::thread_detach_func)GetProcAddress(hModule, "mono_thread_detach");
            runtime_invoke = (typedefs::runtime_invoke_func)GetProcAddress(hModule, "mono_runtime_invoke");
            object_get_class = (typedefs::object_get_class_func)GetProcAddress(hModule, "mono_object_get_class");
            class_get_name = (typedefs::class_get_name_func)GetProcAddress(hModule, "mono_class_get_name");
            class_get_namespace = (typedefs::class_get_namespace_func)GetProcAddress(hModule, "mono_class_get_namespace");
            class_get_method_from_name = (typedefs::class_get_method_from_name_func)GetProcAddress(hModule, "mono_class_get_method_from_name");
            domain_foreach = (typedefs::domain_foreach_func)GetProcAddress(hModule, "mono_domain_foreach");
            compile_method = (typedefs::compile_method_func)GetProcAddress(hModule, "mono_compile_method");
            domain_get = (typedefs::domain_get_func)GetProcAddress(hModule, "mono_domain_get");
            domain_set = (typedefs::domain_set_func)GetProcAddress(hModule, "mono_domain_set");
            class_from_name = (typedefs::class_from_name_func)GetProcAddress(hModule, "mono_class_from_name");
            domain_open_assembly = (typedefs::domain_open_assembly_func)GetProcAddress(hModule, "mono_domain_assembly_open");
            assembly_get_image = (typedefs::assembly_get_image_func)GetProcAddress(hModule, "mono_assembly_get_image");
            class_get_field_from_name = (typedefs::class_get_field_from_name_func)GetProcAddress(hModule, "mono_class_get_field_from_name");
            class_vtable = (typedefs::class_vtable_func)GetProcAddress(hModule, "mono_class_vtable");
            field_static_get_value = (typedefs::field_static_get_value_func)GetProcAddress(hModule, "mono_field_static_get_value");
            field_get_value_object = (typedefs::field_get_value_object_func)GetProcAddress(hModule, "mono_field_get_value_object");
            domain_get_friendly_name = (typedefs::domain_get_friendly_name_func)GetProcAddress(hModule, "mono_domain_get_friendly_name");
            array_new = (typedefs::array_new_func)GetProcAddress(hModule, "mono_array_new");
            array_addr_with_size = (typedefs::array_addr_with_size_func)GetProcAddress(hModule, "mono_array_addr_with_size");
            get_byte_class = (typedefs::get_byte_class_func)GetProcAddress(hModule, "mono_get_byte_class");
            object_to_string = (typedefs::object_to_string_func)GetProcAddress(hModule, "mono_object_to_string");

            int index;
            if ((index = Validate()) != -1)
            {
                throw std::runtime_error(std::format("Failed to initialize Mono methods ptr at index: {}", index));
            }
        }

        // hacky way to validate that all function pointers are initialized
        int Validate() const
        {
            size_t size = sizeof(Impl);
            void** ptr = (void**) this;

            for (size_t i = 0 ; i < size / sizeof(void*); i++)
            {
                if (ptr[i] == nullptr)
                {
                    return i;
                }
            }

            return -1;
        }
    };

    MonoMethods& MonoMethods::GetInstance()
    {
        static MonoMethods instance;
        return instance;
    }

    MonoMethods::MonoMethods()
        : m_Impl(std::make_unique<Impl>())
    {
    }

    MonoString* MonoMethods::string_new(MonoDomain* domain, const char* str)
    {
        return m_Impl->string_new(domain, str);
    }

    char* MonoMethods::string_to_utf8(MonoString* str)
    {
        return m_Impl->string_to_utf8(str);
    }

    void MonoMethods::free(void* ptr)
    {
        m_Impl->free(ptr);
    }

    void MonoMethods::domain_foreach(void (*func)(MonoDomain* domain, void* user_data), void* user_data)
    {
        m_Impl->domain_foreach(func, user_data);
    }

    MonoDomain* MonoMethods::get_root_domain()
    {
        return m_Impl->get_root_domain();
    }

    MonoDomain* MonoMethods::thread_attach(MonoDomain* domain)
    {
        return m_Impl->thread_attach(domain);
    }

    void MonoMethods::thread_detach(MonoDomain* domain)
    {
        m_Impl->thread_detach(domain);
    }

    MonoObject* MonoMethods::runtime_invoke(MonoMethod* method, MonoObject* obj, void** params, MonoObject** exc)
    {
        return m_Impl->runtime_invoke(method, obj, params, exc);
    }

    MonoClass* MonoMethods::object_get_class(MonoObject* obj)
    {
        return m_Impl->object_get_class(obj);
    }

    const char* MonoMethods::class_get_name(MonoClass* klass)
    {
        return m_Impl->class_get_name(klass);
    }

    const char* MonoMethods::class_get_namespace(MonoClass* klass)
    {
        return m_Impl->class_get_namespace(klass);
    }

    MonoMethod* MonoMethods::class_get_method_from_name(MonoClass* klass, const char* name, int param_count)
    {
        return m_Impl->class_get_method_from_name(klass, name, param_count);
    }

    void* MonoMethods::compile_method(MonoMethod* method)
    {
        return m_Impl->compile_method(method);
    }

    MonoDomain* MonoMethods::domain_get()
    {
        return m_Impl->domain_get();
    }

    void MonoMethods::domain_set(MonoDomain* domain)
    {
        m_Impl->domain_set(domain);
    }

    MonoAssembly* MonoMethods::domain_open_assembly(MonoDomain* domain, const char* name)
    {
        return m_Impl->domain_open_assembly(domain, name);
    }

    MonoImage* MonoMethods::assembly_get_image(MonoAssembly* assembly)
    {
        return m_Impl->assembly_get_image(assembly);
    }

    MonoClass* MonoMethods::class_from_name(MonoImage* image, const char* name_space, const char* name)
    {
        return m_Impl->class_from_name(image, name_space, name);
    }

    MonoField* MonoMethods::class_get_field_from_name(MonoClass* klass, const char* name)
    {
        return m_Impl->class_get_field_from_name(klass, name);
    }

    void* MonoMethods::class_vtable(MonoDomain* domain, MonoClass* klass)
    {
        return m_Impl->class_vtable(domain, klass);
    }

    void MonoMethods::field_static_get_value(void* vtable, MonoField* field, void** value)
    {
        m_Impl->field_static_get_value(vtable, field, value);
    }

    MonoObject* MonoMethods::field_get_value_object(MonoDomain* domain, MonoField* field, MonoObject* obj)
    {
        return m_Impl->field_get_value_object(domain, field, obj);
    }

    const char* MonoMethods::domain_get_friendly_name(MonoDomain* domain)
    {
        return m_Impl->domain_get_friendly_name(domain);
    }

    MonoClass* MonoMethods::get_byte_class()
    {
        return m_Impl->get_byte_class();
    }

    MonoArray* MonoMethods::array_new(MonoDomain* domain, MonoClass* eclass, uintptr_t n)
    {
        return m_Impl->array_new(domain, eclass, n);
    }

    void* MonoMethods::array_addr_with_size(void* array, int size, uintptr_t idx)
    {
        return m_Impl->array_addr_with_size(array, size, idx);
    }

    MonoString* MonoMethods::object_to_string(MonoObject* obj, MonoObject** exc)
    {
        return m_Impl->object_to_string(obj, exc);
    }

    void MonoMethods::print_exception(MonoObject* exc)
    {
        if (exc)
        {
            MonoObject* newExc = nullptr;
            MonoObject* strObj = object_to_string(exc, &newExc);
            if (strObj)
            {
                char* str = string_to_utf8((MonoString*)strObj);
                if (str)
                {
                    printf("Mono Exception: %s\n", str);
                    free(str);
                }
            }
        }
    }
}