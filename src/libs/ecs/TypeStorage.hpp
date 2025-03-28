#pragma once

#include "ecs/TypeTraits.hpp"
#include "absl/container/flat_hash_map.h"

namespace RR::Ecs
{
    class Components
    {
    public:
        template <typename T>
        TypeId Register()
        {
            const auto descriptor = GetTypeDescriptor<T>;
            types.insert({descriptor.id, descriptor});
            return descriptor.id;
        }

        const TypeDescriptor& operator[](TypeId id) const { return types[id]; }

    private:
        absl::flat_hash_map<TypeId, TypeDescriptor> types;
    };
}