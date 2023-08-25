#pragma once

#include "rfx/core/UnownedStringSlice.hpp"
#include "stl/vector_map.hpp"

namespace RR
{
    namespace Common
    {
        enum class RResult;
    }

    namespace Rfx
    {
        using RResult = Common::RResult;

        struct CompileContext;
        struct SourceLocation;
        class DiagnosticSink;
        struct Token;

        struct JSONValue
        {
        private:
            struct comporator
            {
                int compareStr(const UnownedStringSlice& a, const UnownedStringSlice& b) const
                {
                    for (auto aIt = a.Begin(), bIt = b.Begin();
                         aIt != a.End() && bIt != b.End();
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
                    const bool isArrayKeys = (a.Begin() == a.End() && b.Begin() == b.End());
                    return isArrayKeys ? a.Begin() < b.Begin() : compare(a, b) < 0;
                }
            };

        public:
            // Potentially there is the best container for both array and object values. See tsl::ordered_map
            using Container = stl::vector_map<UnownedStringSlice, JSONValue, comporator>;
            using const_iterator = Container::const_iterator;
            using iterator = Container::iterator;
            using reverse_iterator = Container::reverse_iterator;
            using const_reverse_iterator = Container::const_reverse_iterator;

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
            JSONValue() = default;
            JSONValue(const JSONValue& other) { copy(other); }
            JSONValue(JSONValue&& other) noexcept { swap(other); }
            ~JSONValue()
            {
                if (IsObjectLike())
                    container = nullptr;
            }

            JSONValue& operator=(const JSONValue& other) { return copy(other); }
            JSONValue& operator=(JSONValue&& other) noexcept { return swap(other); }

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

            bool IsObjectLike() const { return type == Type::Object || type == Type::Array; }
            bool IsObject() const { return type == Type::Object; }
            bool IsArray() const { return type == Type::Array; }

            static JSONValue MakeBool(bool inValue)
            {
                JSONValue value;
                value.type = Type::Bool;
                value.boolValue = inValue;
                return value;
            }

            static JSONValue MakeFloat(double inValue)
            {
                JSONValue value;
                value.type = Type::Float;
                value.floatValue = inValue;
                return value;
            }

            static JSONValue MakeInt(int64_t inValue)
            {
                JSONValue value;
                value.type = Type::Integer;
                value.intValue = inValue;
                return value;
            }

            static JSONValue MakeNull()
            {
                JSONValue value;
                value.type = Type::Null;
                return value;
            }

            static JSONValue MakeString(const UnownedStringSlice& inValue)
            {
                JSONValue value;
                value.type = Type::String;
                value.stringValue = inValue;
                return value;
            }

            static JSONValue JSONValue::MakeEmptyArray()
            {
                JSONValue value;
                value.type = Type::Array;
                value.container = std::make_shared<Container>();
                return value;
            }

            static JSONValue JSONValue::MakeEmptyObject()
            {
                JSONValue value;
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
                    case Type::Invalid: ASSERT_MSG(false, "Invalid value");
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
                    case Type::Invalid: ASSERT_MSG(false, "Invalid value");
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
                    case Type::Invalid: ASSERT_MSG(false, "Invalid value");
                    default: return 0.0;
                }
            }

        public:
            const JSONValue& Find(const UnownedStringSlice& key) const
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
            friend class JSONBuilder;

            const JSONValue& append(const JSONValue& value) { return append(JSONValue(value)); }
            const JSONValue& append(JSONValue&& value)
            {
                ASSERT_MSG(IsArray(), "Append requires ArrayValue");

                if (!IsArray())
                    return nullValue();

                // Encode index to pointer
                U8Char* ptr = (U8Char*)(container->size());
                return container->emplace_back_unsorted(UnownedStringSlice(ptr, ptr), std::move(value)).second;
            }

            JSONValue& operator[](const UnownedStringSlice& key)
            {
                if (!IsObject())
                    return nullValue();

                return container->emplace(key, JSONValue {}).first->second;
            }

            JSONValue& nullValue() const
            {
                static JSONValue null = {};
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
            JSONValue& copy(const JSONValue& other)
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

            JSONValue& swap(JSONValue& other)
            {
                std::swap(data, other.data);
                std::swap(type, other.type);
                return *this;
            }
        };

        class JSONBuilder
        {
        public:
            RResult StartObject(const Token& token);
            void EndObject();
            RResult StartArray(const Token& token);
            void EndArray();
            void BeginInrehitance();
            RResult AddParent(const Token& parent);

            RResult AddKey(const Token& key);
            RResult AddValue(const Token& token);
            /// Get the root value. Will be set after valid construction
            const JSONValue& GetRootValue() const { return root_; }

            JSONBuilder(const std::shared_ptr<CompileContext>& context);

        private:
            DiagnosticSink& getSink() const;
            JSONValue& currentValue() { return stack_.back(); }

            int64_t tokenToInt(const Token& token, int radix);
            double tokenToFloat(const Token& token);

            RResult add(const Token& token, const JSONValue& value) { return add(token, JSONValue(value)); }
            RResult add(const Token& token, JSONValue&& value);

        private:
            enum class Expect : uint8_t
            {
                ObjectKey,
                ObjectValue,
                ArrayValue,
                Parent
            };

            Expect expect_;
            UnownedStringSlice key_;
            std::vector<JSONValue::Container*> parents_;
            std::vector<JSONValue> stack_;
            JSONValue root_;
            std::shared_ptr<CompileContext> context_;
        };

        U8String JSONValueTypeToString(JSONValue::Type type);
    }
}

template <>
struct fmt::formatter<RR::Rfx::JSONValue::Type> : formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(RR::Rfx::JSONValue::Type type, FormatContext& ctx)
    {
        return formatter<string_view>::format(RR::Rfx::JSONValueTypeToString(type), ctx);
    }
};