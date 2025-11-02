#pragma once

#include "eastl/vector_map.h"
#include "eastl/string_view.h"

namespace RR::EffectLibrary
{
    struct ShaderDefaultData
    {
        ShaderDefaultData() = default;

        ShaderDefaultData(int32_t value) : isInitialized(true) { data.intValue = value; }
        ShaderDefaultData(bool value) : isInitialized(true) { data.boolValue = value; }

        bool isInitialized = false;
        union Data
        {
            float floatValue;
            int32_t intValue;
            bool boolValue;
        } data;
    };


    struct Foo;

    struct ShaderField
    {
        enum class Type : uint8_t
        {
            Float,
            Int,
            Bool,
            Struct
        };

        ShaderField();
        ~ShaderField();
       // ShaderField(Type type) : type(type) { }
      //  ShaderField(Type type, ShaderDefaultData defaultValue) : type(type), defaultValue(defaultValue) { }

      //  void emplace(eastl::string_view name, ShaderField&& field);

        Type type;
        ShaderDefaultData defaultValue;
   //     eastl::unique_ptr<std::vector<eastl::string_view, ShaderField> > childs;
    };



/*
    // Implementation moved outside the struct to avoid template instantiation issues
    inline void ShaderField::emplace(eastl::string_view name, ShaderField&& field)
    {
        if (!childs)
            childs = eastl::make_unique<eastl::vector_map<eastl::string_view, ShaderField>>();

        childs->emplace(name, eastl::move(field));
    }*/

    struct ShaderReflectionData
    {
        ShaderField root;
    };

}