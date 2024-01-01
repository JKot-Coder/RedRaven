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
                const auto aLen = a.GetLength();
                const auto bLen = b.GetLength();
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
            if (IsObjectLike())
                container = nullptr;
        }

        RSONValue& operator=(const RSONValue& other) { return copy(other); }
        RSONValue& operator=(RSONValue&& other) noexcept { return swap(other); }

        iterator begin() noexcept { return IsObjectLike() ? container->begin() : iterator {}; }
        const_iterator begin() const noexcept { return cbegin(); }
        const_iterator cbegin() const noexcept { return IsObjectLike() ? container->cbegin() : const_iterator {}; }

        iterator end() noexcept { return IsObjectLike() ? container->end() : iterator {}; }
        const_iterator end() const noexcept { return cend(); }
        const_iterator cend() const noexcept { return IsObjectLike() ? container->cend() : const_iterator {}; }

        reverse_iterator rbegin() noexcept { return IsObjectLike() ? container->rbegin() : reverse_iterator {}; }
        const_reverse_iterator rbegin() const noexcept { return crbegin(); }
        const_reverse_iterator crbegin() const noexcept { return IsObjectLike() ? container->crbegin() : const_reverse_iterator {}; }

        reverse_iterator rend() noexcept { return IsObjectLike() ? container->rend() : reverse_iterator {}; }
        const_reverse_iterator rend() const noexcept { return crend(); }
        const_reverse_iterator crend() const noexcept { return IsObjectLike() ? container->crend() : const_reverse_iterator {}; }

        bool IsValid() const { return type != Type::Invalid; }
        bool IsObjectLike() const { return type == Type::Object || type == Type::Array; }
        bool IsObject() const { return type == Type::Object; }
        bool IsArray() const { return type == Type::Array; }

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

        static RSONValue MakeString(const UnownedStringSlice& inValue)
        {
            RSONValue value;
            value.type = Type::String;
            value.stringValue = inValue;
            return value;
        }

        static RSONValue RSONValue::MakeEmptyArray()
        {
            RSONValue value;
            value.type = Type::Array;
            value.container = std::make_shared<Container>();
            return value;
        }

        static RSONValue RSONValue::MakeEmptyObject()
        {
            RSONValue value;
            value.type = Type::Object;
            value.container = std::make_shared<Container>();
            return value;
        }

        bool AsBool() const
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

        int64_t AsInteger() const
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

        double AsFloat() const
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

    public:
        const RSONValue& Find(const UnownedStringSlice& key) const
        {
            if (IsObject())
            {
                const auto iterator = container->find(key);
                if (iterator != container->end())
                    return iterator->second;
            }

            return nullValue();
        }

        const bool Contains(const UnownedStringSlice& key) const
        {
            if (IsObject() && (container->find(key) != container->end()))
                return true;

            return false;
        }

    private:
        friend class RSONBuilder;

        const RSONValue& append(const RSONValue& value) { return append(RSONValue(value)); }
        const RSONValue& append(RSONValue&& value)
        {
            ASSERT_MSG(IsArray(), "Append requires ArrayValue");

            if (!IsArray())
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

        RSONValue& operator[](const UnownedStringSlice& key)
        {
            if (!IsObject())
                return nullValue();

            return container->emplace(key, RSONValue {}).first->second;
        }

        RSONValue& nullValue() const
        {
            static RSONValue null = {};
            return null;
        }

    public:
        Type type = Type::Invalid;

        union
        {
            double floatValue;
            int64_t intValue;
            UnownedStringSlice stringValue;
            bool boolValue;
            std::shared_ptr<Container> container {};
            std::array<uint8_t, 16> data;
        };
        static_assert(sizeof(data) >= sizeof(UnownedStringSlice) &&
                      sizeof(data) >= sizeof(std::shared_ptr<void>));

    private:
        RSONValue& copy(const RSONValue& other)
        {
            if (IsObjectLike())
                container = nullptr;

            if (other.IsObjectLike())
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