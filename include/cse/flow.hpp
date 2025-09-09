#pragma once
#include <cse/mono.hpp>
#include <cstdint>
#include <vector>
#include <utility>

namespace cse
{
    struct guid_t
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t  data4[8];
    };

    MonoObject* HookedCreateObjectInstance(guid_t guid, guid_t iid);
    MonoObject* UnmarshalIUnknown(void* unknown);

    // For handling MonoScriptRuntime objects from CreateObjectInstance
    void HandleMSR(MonoObject* msr);

    /**
     * Find all internal MonoScriptRuntime managers and their domains.
     * @return A vector of pairs, each containing a MonoScriptRuntime object and its associated MonoDomain.
     */
    std::vector<std::pair<MonoObject*, MonoDomain*>> FindAllInternalManagers();
}