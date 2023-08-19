#pragma once

#include "rfx/core/UnownedStringSlice.hpp"
#include <unordered_map>

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
        class JSONContainer;
        class DiagnosticSink;
        struct Token;

        struct JSONValue
        {
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
            JSONValue() = default;
            JSONValue(const JSONValue& other) { copy(other); }
            JSONValue(JSONValue&& other) noexcept { swap(other); }
            ~JSONValue();

            JSONValue& operator=(const JSONValue& other) { return copy(other); }
            JSONValue& operator=(JSONValue&& other) noexcept { return swap(other); }

            bool isContainer() const { return type == Type::Object || type == Type::Array; }

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

            static JSONValue MakeEmptyArray();
            static JSONValue MakeEmptyObject();

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

            Type type = Type::Invalid;

            union
            {
                double floatValue;
                int64_t intValue;
                UnownedStringSlice stringValue;
                bool boolValue;
                std::shared_ptr<JSONContainer> container = {};
                std::array<uint8_t, 16> data;
            };
            static_assert(sizeof(data) >= sizeof(UnownedStringSlice) &&
                          sizeof(data) >= sizeof(std::shared_ptr<JSONContainer>));

        private:
            JSONValue& copy(const JSONValue& other)
            {
                if (isContainer())
                    container = nullptr;

                if (other.isContainer())
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

        class JSONContainer
        {
        public:
            enum class Type
            {
                Invalid,
                Object,
                Array,
            };

            JSONContainer(Type type) : type_(type) { }
            Type GetType() const { return type_; }
            void AddKeyValue(const UnownedStringSlice& stringSlice, const JSONValue& value);
            void AddValue(const JSONValue& value);
            JSONValue Find(const UnownedStringSlice& name) const;
            bool IsObjectValueExist(const UnownedStringSlice& stringSlice);

            void InheriteFrom(const std::vector<std::shared_ptr<JSONContainer>>& parents);
            static std::shared_ptr<JSONContainer> MakeObject()
            {
                return std::make_shared<JSONContainer>(Type::Object);
            }

            static std::shared_ptr<JSONContainer> MakeArray()
            {
                return std::make_shared<JSONContainer>(Type::Array);
            }

        private:
            Type type_ = Type::Invalid;
            std::vector<JSONValue> arrayValues_;
            std::unordered_map<UnownedStringSlice, JSONValue> objectValues_;
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

            JSONBuilder(JSONValue& rootValue, const std::shared_ptr<CompileContext>& context);

        private:
            DiagnosticSink& getSink() const;
            JSONContainer& currentContainer() { return *stack_.back(); }

            int64_t tokenToInt(const Token& token, int radix);
            double tokenToFloat(const Token& token);

            RResult add(const Token& token, const JSONValue& value);

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
            std::vector<std::shared_ptr<JSONContainer>> parents_;
            std::vector<std::shared_ptr<JSONContainer>> stack_;
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