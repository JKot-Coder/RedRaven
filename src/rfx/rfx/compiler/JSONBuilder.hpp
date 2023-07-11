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
            };

            std::shared_ptr<JSONContainer> container;
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

            Type GetType() const { return type_; }
            void AddKeyValue(const UnownedStringSlice& stringSlice, const JSONValue& value);
            void AddValue(const JSONValue& value);
            JSONValue Find(const UnownedStringSlice& name) const;
            bool IsObjectValueExist(const UnownedStringSlice& stringSlice);

            void InheriteFrom(const std::vector<std::shared_ptr<JSONContainer>>& parents);

            static std::shared_ptr<JSONContainer> MakeObject()
            {
                auto value = std::make_shared<JSONContainer>();
                value->type_ = Type::Object;
                return value;
            }

            static std::shared_ptr<JSONContainer> MakeArray()
            {
                auto value = std::make_shared<JSONContainer>();
                value->type_ = Type::Array;
                return value;
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
            // const JSONValue& getRootValue() const { return m_rootValue; }

            JSONBuilder(const std::shared_ptr<CompileContext>& context);

        private:
            DiagnosticSink& getSink() const;
            std::shared_ptr<JSONContainer> currentContainer() { return stack_.back(); }

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
            std::shared_ptr<JSONContainer> root_;
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