#pragma once

#include "ecs/TypeTraits.hpp"
#include "ska/flat_hash_map.h"

namespace RR::Ecs
{
    class TypeStorage
    {
    public:
        template <typename T>
        TypeId Register()
        {
            const auto descriptor = TypeTraits<T>::descriptor;
            types.insert({descriptor.id, descriptor});
            return descriptor.id;
        }

        TypeDescriptor operator[](TypeId id)
        {
            return types[id];
        }

    private:
        ska::flat_hash_map<TypeId, TypeDescriptor> types;
    };
}