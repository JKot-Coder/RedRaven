#pragma once

#include "ecs/TypeTraits.hpp"
#include "ska/flat_hash_map.h"

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
        ska::flat_hash_map<TypeId, TypeDescriptor> types;
    };
}