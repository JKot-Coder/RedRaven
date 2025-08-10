#pragma once

#include "ComponentTraits.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Ecs::Meta
{
    class Storage;
    struct ComponentInfoBuilder
    {
        ComponentInfoBuilder(Storage& storage, ComponentInfo& componentInfo) : storage(&storage), componentInfo(&componentInfo) { }

        [[nodiscard]] ComponentId id() const { return componentInfo->id; }

    private:
        [[maybe_unused]] Storage* storage;
        ComponentInfo* componentInfo;
    };

    class Storage
    {
    public:
        template <typename T>
        ComponentInfoBuilder Register()
        {
            static auto componentInfo = ComponentInfo::Create<T>();
            ASSERT_MSG(isValid(componentInfo), "Component differs from previous registration!");
            componentsInfo.emplace(componentInfo.id, &componentInfo);
            return ComponentInfoBuilder(*this, componentInfo);
        }

        ComponentInfo& operator[](ComponentId id) { ASSERT(componentsInfo[id]); return *componentsInfo[id]; }
        auto find(ComponentId id) { return componentsInfo.find(id); }
        auto find(ComponentId id) const { return componentsInfo.find(id); }
        auto begin() const { return componentsInfo.begin(); }
        auto end() const { return componentsInfo.end(); }

    private:
        bool isValid(const ComponentInfo& componentInfo)
        {
            auto it = componentsInfo.find(componentInfo.id);
            if (it == componentsInfo.end())
                return true;

            return *(it->second) == componentInfo;
        }
    private:
        absl::flat_hash_map<ComponentId, ComponentInfo*, Ecs::DummyHasher<ComponentId>> componentsInfo;
    };
}