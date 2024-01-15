#pragma once

#include "rfx/core/UnownedStringSlice.hpp"
#include "stl/vector_map.hpp"

namespace RR::Rfx
{
    struct RSONValue
    {
    private:
        struct comporator
        {
            int compareStr(const UnownedStringSlice& a, const UnownedStringSlice& b) const
            {
                for (auto aIt = a.begin(), bIt = b.begin();
                     aIt != a.end() && bIt != b.end();
                     ++aIt, ++bIt)
                {
                    if (*aIt != *bIt)
                        return (*aIt < *bIt) ? -1 : 1;
                }
                return 0;
            }

            int compare(const UnownedStringSlice& a, const UnownedStringSlice& b) const
            {
                const auto aLen = a.length();
                const auto bLen = b.length();
                const int cmp = compareStr(a, b);

                return (cmp != 0 ? cmp : (aLen < bLen ? -1 : (aLen > bLen ? 1 : 0)));
            }

            bool operator()(const UnownedStringSlice& a, const UnownedStringSlice& b) const
            {
                const bool isArrayKeys = (a.begin() == a.end() && b.begin() == b.end());
                return isArrayKeys ? a.begin() < b.begin() : compare(a, b) < 0;
            }
        };

    public:
        // Potentially there is the best container for both array and object values. See tsl::ordered_map
        using Container = stl::vector_map<UnownedStringSlice, RSONValue, comporator>;
        using const_iterator = Container::const_iterator;
        using iterator = Container::iterator;
        using reverse_iterator = Container::reverse_iterator;
        using const_reverse_iterator = Container::const_reverse_iterator;
        using insert_return_type = Container::insert_return_type;

        enum class Type
        {
            Invalid,

            Bool,
            Float,
            Integer,
            Null,
            String,
            Reference,

            Array,
            Object,

            CountOf
        };

    public:
        RSONValue() = default;
        RSONValue(const RSONValue& other) { copy(other); }
        RSONValue(RSONValue&& other) noexcept { swap(other); }
        ~RSONValue()
        {
            if (isObjectLike())
                container = nullptr;
        }

        RSONValue& operator=(const RSONValue& other) { return copy(other); }
        RSONValue& operator=(RSONValue&& other) noexcept { return swap(other); }

        iterator begin() noexcept { return isObjectLike() ? container->begin() : iterator {}; }
        const_iterator begin() const noexcept { return cbegin(); }
        const_iterator cbegin() const noexcept { return isObjectLike() ? container->cbegin() : const_iterator {}; }

        iterator end() noexcept { return isObjectLike() ? container->end() : iterator {}; }
        const_iterator end() const noexcept { return cend(); }
        const_iterator cend() const noexcept { return isObjectLike() ? container->cend() : const_iterator {}; }

        reverse_iterator rbegin() noexcept { return isObjectLike() ? container->rbegin() : reverse_iterator {}; }
        const_reverse_iterator rbegin() const noexcept { return crbegin(); }
        const_reverse_iterator crbegin() const noexcept { return isObjectLike() ? container->crbegin() : const_reverse_iterator {}; }

        reverse_iterator rend() noexcept { return isObjectLike() ? container->rend() : reverse_iterator {}; }
        const_reverse_iterator rend() const noexcept { return crend(); }
        const_reverse_iterator crend() const noexcept { return isObjectLike() ? container->crend() : const_reverse_iterator {}; }

        /// Number of values in array or object
        size_t size() const { return isObjectLike() ? container->size() : 0; }
        bool empty() const { return size() == 0; }

        bool isValid() const { return type != Type::Invalid; }
        bool isNull() const { return type == Type::Null; }
        bool isBool() const { return type == Type::Bool; }
        bool isObjectLike() const { return type == Type::Object || type == Type::Array; }
        bool isObject() const { return type == Type::Object; }
        bool isArray() const { return type == Type::Array; }

        static RSONValue MakeBool(bool inValue)
        {
            RSONValue value;
            value.type = Type::Bool;
            value.boolValue = inValue;
            return value;
        }

        static RSONValue MakeFloat(double inValue)
        {
            RSONValue value;
            value.type = Type::Float;
            value.floatValue = inValue;
            return value;
        }

        static RSONValue MakeInt(int64_t inValue)
        {
            RSONValue value;
            value.type = Type::Integer;
            value.intValue = inValue;
            return value;
        }

        static RSONValue MakeNull()
        {
            RSONValue value;
            value.type = Type::Null;
            return value;
        }

        static RSONValue MakeString(UnownedStringSlice inValue)
        {
            RSONValue value;
            value.type = Type::String;
            value.stringValue = std::move(inValue);
            return value;
        }

        static RSONValue MakeReference(UnownedStringSlice inValue)
        {
            RSONValue value;
            value.type = Type::Reference;
            value.referenceValue.path = std::move(inValue);
            value.referenceValue.reference = nullptr;
            return value;
        }

        static RSONValue MakeEmptyArray()
        {
            RSONValue value;
            value.type = Type::Array;
            value.container = std::make_shared<Container>();
            return value;
        }

        static RSONValue MakeEmptyObject()
        {
            RSONValue value;
            value.type = Type::Object;
            value.container = std::make_shared<Container>();
            return value;
        }

        bool asBool() const
        {
            switch (type)
            {
                case Type::Bool: return boolValue;
                case Type::Null: return false;
                case Type::Integer: return intValue != 0;
                case Type::Float: return floatValue != 0;
                case Type::Invalid: ASSERT_MSG(false, "Invalid value"); return false;
                default: return false;
            }
        };
        bool getBool() const
        {
            ASSERT(type == Type::Bool);
            return boolValue;
        }
        bool getBool(bool default) const { return (type == Type::Bool) ? boolValue : default; }

        int64_t asInteger() const
        {
            switch (type)
            {
                case Type::Bool: return boolValue;
                case Type::Null: return 0;
                case Type::Integer: return intValue;
                case Type::Float: return int64_t(floatValue);
                case Type::Invalid: ASSERT_MSG(false, "Invalid value"); return 0;
                default: return 0;
            }
        }
        int64_t getInteger() const
        {
            ASSERT(type == Type::Integer);
            return intValue;
        }
        int64_t getInteger(int64_t default) const { return (type == Type::Integer) ? intValue : default; }

        double asFloat() const
        {
            switch (type)
            {
                case Type::Bool: return boolValue;
                case Type::Null: return 0.0;
                case Type::Integer: return double(intValue);
                case Type::Float: return floatValue;
                case Type::Invalid: ASSERT_MSG(false, "Invalid value"); return 0.0;
                default: return 0.0;
            }
        }
        double getFloat() const
        {
            ASSERT(type == Type::Float);
            return floatValue;
        }
        double getFloat(double default) const { return (type == Type::Float) ? floatValue : default; }

        U8String asString() const
        {
            switch (type)
            {
                case Type::Bool: return boolValue ? "true" : "false";
                case Type::Null: return "null";
                case Type::Integer: return std::to_string(intValue);
                case Type::Float: return std::to_string(floatValue);
                case Type::String: return stringValue.asString();
                case Type::Reference: return referenceValue.path.asString();
                case Type::Invalid: ASSERT_MSG(false, "Invalid value"); return "";
                default: return "";
            }
        }
        U8String getString() const
        {
            ASSERT(type == Type::String);
            return stringValue.asString();
        }
        U8String getString(U8String default) const { return (type == Type::String) ? stringValue.asString() : default; }

        template <typename EnumType>
        EnumType getEnum(EnumType defaultValue)
        {
            static_assert(std::is_enum_v<EnumType>, "Epected enum type");
            return static_cast<EnumType>(getInteger(static_cast<int64_t>(defaultValue)));
        }

    public:
        template <typename T, U8Char Delimiter>
        const RSONValue& find(const StringSplit<T, Delimiter>& stringSplit) const
        {
            RSONValue const* current = this;

            for (const auto& split : stringSplit)
            {
                if (current->isObject())
                {
                    const auto iterator = current->container->find(split.slice());
                    if (iterator != current->container->end())
                    {
                        current = &iterator->second;
                        continue;
                    }
                }

                return nullValue();
            }

            return *current;
        }

        const RSONValue& find(UnownedStringSlice key) const
        {
            return find(StringSplit<UnownedStringSlice, '.'>(key));
        }

        const bool contains(UnownedStringSlice key) const
        {
            if (isObject() && (container->find(key) != container->end()))
                return true;

            return false;
        }

        const RSONValue& operator[](UnownedStringSlice key) const
        {
            if (!isObject())
                return nullValue();

            return container->emplace(std::move(key), RSONValue {}).first->second;
        }

    private:
        friend class RSONBuilder;

        const RSONValue& append(const RSONValue& value) { return append(RSONValue(value)); }
        const RSONValue& append(RSONValue&& value)
        {
            ASSERT_MSG(isArray(), "Append requires ArrayValue");

            if (!isArray())
                return nullValue();

            // Encode index to pointer
            U8Char* ptr = (U8Char*)(container->size());
            return container->emplace_back_unsorted(UnownedStringSlice(ptr, ptr), std::move(value)).second;
        }

        template <class... Args>
        insert_return_type emplace(Args&&... args)
        {
            Container::value_type value(std::forward<Args>(args)...);
            return container->insert(std::move(value));
        }

        RSONValue& nullValue() const
        {
            static RSONValue null = {};
            return null;
        }

    public:
        Type type = Type::Invalid;

        struct ReferenceValue
        {
            UnownedStringSlice path;
            RSONValue const* reference;
        };

        union
        {
            double floatValue;
            int64_t intValue;
            ReferenceValue referenceValue;
            UnownedStringSlice stringValue;
            bool boolValue;
            std::shared_ptr<Container> container {};
            std::array<uint8_t, 24> data;
        };

        static_assert(sizeof(data) == sizeof(ReferenceValue) &&
                      sizeof(data) >= sizeof(std::shared_ptr<Container>));

    private:
        RSONValue& copy(const RSONValue& other)
        {
            if (isObjectLike())
                container = nullptr;

            if (other.isObjectLike())
                container = other.container;
            else
                data = other.data;

            type = other.type;
            return *this;
        }

        RSONValue& swap(RSONValue& other)
        {
            std::swap(data, other.data);
            std::swap(type, other.type);
            return *this;
        }
    };

    U8String RSONValueTypeToString(RSONValue::Type type);
}

template <>
struct fmt::formatter<RR::Rfx::RSONValue::Type> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::RSONValue::Type type, FormatContext& ctx)
    {
        return formatter<string_view>::format(RR::Rfx::RSONValueTypeToString(type), ctx);
    }
};