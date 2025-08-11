#pragma once

#include "ComponentTraits.hpp"

#include "absl/container/flat_hash_map.h"

namespace RR::Ecs::Meta
{
    class Storage;

    template <typename T, bool = std::is_class_v<T>>
    struct ComponentInfoBuilderImpl;

    template <typename T>
    struct ComponentInfoBuilderImpl<T, false>
    {
    public:
        ComponentInfoBuilderImpl(Storage& storage, ComponentInfo& componentInfo) : storage(&storage), componentInfo(&componentInfo) { }

    protected:
        Storage* storage;
        ComponentInfo* componentInfo;
    };

    template <typename T>
    struct ComponentInfoBuilder;

    template <typename Class>
    struct ComponentInfoBuilderImpl<Class, true>
    {
    public:
        ComponentInfoBuilderImpl(Storage& storage, ComponentInfo& componentInfo) : storage(&storage), componentInfo(&componentInfo) { }
        template <typename Field>
        ComponentInfoBuilder<Class> Element(const char* name, Field Class::* member)
        {
            if (clearElementsNeeded)
            {
                componentInfo->elements.clear();
                clearElementsNeeded = false;
            }

            uint16_t offset = reinterpret_cast<std::size_t>(
                &(reinterpret_cast<Class*>(0)->*member)
            );

            auto fieldInfo = storage->Register<Field>();
            componentInfo->elements.emplace_back(name, offset, fieldInfo.Info());
            return static_cast<ComponentInfoBuilder<Class>&>(*this);
        }

    protected:
        bool clearElementsNeeded = true;
        Storage* storage;
        ComponentInfo* componentInfo;
    };

    template <typename T>
    struct ComponentInfoBuilder : public ComponentInfoBuilderImpl<T>
    {
        ComponentInfoBuilder(Storage& storage, ComponentInfo& componentInfo) : ComponentInfoBuilderImpl<T>(storage, componentInfo) { }

        [[nodiscard]] ComponentId Id() const { return this->componentInfo->id; }
        [[nodiscard]] ComponentInfo& Info() const { return *this->componentInfo; }
    };

    class Storage
    {
    public:
        template <typename T>
        ComponentInfoBuilder<T> Register()
        {
            static auto componentInfo = ComponentInfo::Create<T>();
            ASSERT_MSG(isValid(componentInfo), "Component differs from previous registration!");
            componentsInfo.emplace(componentInfo.id, &componentInfo);
            return ComponentInfoBuilder<T>(*this, componentInfo);
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