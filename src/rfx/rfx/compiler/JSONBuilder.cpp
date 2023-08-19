#include "JSONBuilder.hpp"

#include "DiagnosticCore.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/Token.hpp"
#include "rfx/core/SourceLocation.hpp"

namespace RR::Rfx
{
    namespace
    {
        UnownedStringSlice unquotingToken(const Token& token)
        {
            if (token.type == Token::Type::StringLiteral || token.type == Token::Type::CharLiteral)
                return UnownedStringSlice(token.stringSlice.Begin() + 1, token.stringSlice.End() - 1);

            return token.stringSlice;
        }
    }

    U8String JSONValueTypeToString(JSONValue::Type type)
    {
        static_assert(int(JSONValue::Type::CountOf) == 8);
        switch (type)
        {
            case JSONValue::Type::Invalid: return "Invalid";

            case JSONValue::Type::Bool: return "Bool";
            case JSONValue::Type::Float: return "Float";
            case JSONValue::Type::Integer: return "Integer";
            case JSONValue::Type::Null: return "Null";
            case JSONValue::Type::String: return "String";

            case JSONValue::Type::Array: return "Array";
            case JSONValue::Type::Object: return "Object";

            default:
                ASSERT(!"unexpected");
                return "<uknown>";
        }
    }

    JSONValue::~JSONValue()
    {
        if(isContainer())
            container = nullptr;
    }

    JSONValue JSONValue::MakeEmptyArray()
    {
        JSONValue value;
        value.type = Type::Array;
        value.container = JSONContainer::MakeArray();
        return value;
    }

    JSONValue JSONValue::MakeEmptyObject()
    {
        JSONValue value;
        value.type = Type::Object;
        value.container = JSONContainer::MakeObject();
        return value;
    }

    void JSONContainer::AddValue(const JSONValue& value)
    {
        ASSERT(type_ == Type::Array);
        arrayValues_.push_back(value);
    }

    void JSONContainer::AddKeyValue(const UnownedStringSlice& key, const JSONValue& value)
    {
        ASSERT(type_ == Type::Object);
        const bool emplaced = objectValues_.emplace(key, value).second;
        UNUSED(emplaced);
        ASSERT(emplaced);
    }

    JSONValue JSONContainer::Find(const UnownedStringSlice& key) const
    {
        ASSERT(type_ == Type::Object);
        auto result = objectValues_.find(key);
        return result != objectValues_.end() ? result->second : JSONValue();
    }

    bool JSONContainer::IsObjectValueExist(const UnownedStringSlice& key)
    {
        ASSERT(type_ == Type::Object);
        return objectValues_.find(key) != objectValues_.end();
    }

    void JSONContainer::InheriteFrom(const std::vector<std::shared_ptr<JSONContainer>>& parents)
    {
        ASSERT(type_ == Type::Object);

        for (const auto& parent : parents)
        {
            ASSERT(parent);
            ASSERT(parent->type_ == Type::Object);
            objectValues_.insert(parent->objectValues_.begin(), parent->objectValues_.end());
        }
    }

    JSONBuilder::JSONBuilder(JSONValue& rootValue, const std::shared_ptr<CompileContext>& context) : expect_(Expect::ObjectKey),
                                                                                                     context_(context)
    {
        rootValue.type = JSONValue::Type::Object;
        rootValue.container = JSONContainer::MakeObject();
        root_ = rootValue;
        stack_.emplace_back(rootValue.container);
    }

    DiagnosticSink& JSONBuilder::getSink() const { return context_->sink; }

    RResult JSONBuilder::StartObject(const Token& token)
    {
        const auto value = JSONValue::MakeEmptyObject();
        RR_RETURN_ON_FAIL(add(token, value));
        stack_.emplace_back(value.container);
        expect_ = Expect::ObjectKey;
        return RResult::Ok;
    }

    void JSONBuilder::EndObject()
    {
        ASSERT(currentContainer().GetType() == JSONContainer::Type::Object);
        ASSERT(expect_ == Expect::ObjectKey);
        currentContainer().InheriteFrom(parents_);
        stack_.pop_back();
        key_.Reset();
        parents_.clear();
        expect_ = currentContainer().GetType() == JSONContainer::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;
    }

    RResult JSONBuilder::StartArray(const Token& token)
    {
        const auto value = JSONValue::MakeEmptyArray();
        RR_RETURN_ON_FAIL(add(token, value));
        stack_.emplace_back(value.container);
        expect_ = Expect::ArrayValue;
        return RResult::Ok;
    }

    void JSONBuilder::EndArray()
    {
        ASSERT(currentContainer().GetType() == JSONContainer::Type::Array);
        ASSERT(expect_ == Expect::ArrayValue);
        stack_.pop_back();
        key_.Reset();

        expect_ = currentContainer().GetType() == JSONContainer::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;
    }

    void JSONBuilder::BeginInrehitance()
    {
        ASSERT(currentContainer().GetType() == JSONContainer::Type::Object);
        ASSERT(expect_ == Expect::ObjectValue);
        ASSERT(parents_.size() == 0);
        expect_ = Expect::Parent;
    }

    RResult JSONBuilder::AddParent(const Token& parent)
    {
        ASSERT(expect_ == Expect::Parent);
        ASSERT(parent.type == Token::Type::StringLiteral || parent.type == Token::Type::Identifier);
        auto parentName = unquotingToken(parent);

        auto value = root_.container->Find(parentName);
        switch (value.type)
        {
            case JSONValue::Type::Object:
            {
                parents_.push_back(value.container);
                break;
            }
            case JSONValue::Type::Invalid:
            {
                getSink().Diagnose(parent, Diagnostics::undeclaredIdentifier, parentName);
                return RResult::NotFound;
            }
            default:
            {
                getSink().Diagnose(parent, Diagnostics::invalidParentType, parentName, value.type);
                return RResult::Fail;
            }
        }

        return RResult::Ok;
    }

    RResult JSONBuilder::AddKey(const Token& key)
    {
        ASSERT(expect_ == Expect::ObjectKey);
        ASSERT(key.type == Token::Type::StringLiteral || key.type == Token::Type::Identifier);
        auto keyName = unquotingToken(key);

        if (currentContainer().IsObjectValueExist(keyName))
        {
            getSink().Diagnose(key, Diagnostics::duplicateKey, keyName);
            return RResult::AlreadyExist;
        }

        expect_ = Expect::ObjectValue;
        key_ = keyName;

        return RResult::Ok;
    }

    RResult JSONBuilder::AddValue(const Token& token)
    {
        switch (token.type)
        {
            case Token::Type::StringLiteral:
            case Token::Type::CharLiteral:
            case Token::Type::Identifier:
            {
                UnownedStringSlice stringSlice = unquotingToken(token);
                if (stringSlice == "null")
                    return add(token, JSONValue::MakeNull());
                else if (stringSlice == "true")
                    return add(token, JSONValue::MakeBool(true));
                else if (stringSlice == "false")
                    return add(token, JSONValue::MakeBool(false));

                return add(token, JSONValue::MakeString(stringSlice));
            }
            case Token::Type::IntegerLiteral:
                return add(token, JSONValue::MakeInt(tokenToInt(token, 0)));
            case Token::Type::FloatingPointLiteral:
                return add(token, JSONValue::MakeFloat(tokenToFloat(token)));
            default:
            {
                getSink().Diagnose(token, Diagnostics::unexpectedToken, token.type);
                return RResult::Fail;
            }
        }
    }

    RResult JSONBuilder::add(const Token& token, const JSONValue& value)
    {
        switch (expect_)
        {
            case Expect::ArrayValue: currentContainer().AddValue(value); return RResult::Ok;
            case Expect::Parent:
                if (value.type != JSONValue::Type::Object)
                {
                    getSink().Diagnose(token, Diagnostics::invalidTypeForInheritance, key_, value.type);
                    return RResult::Fail;
                }
                [[fallthrough]];
            case Expect::ObjectValue:
                currentContainer().AddKeyValue(key_, value);
                key_.Reset();
                expect_ = Expect::ObjectKey;
                return RResult::Ok;
            default: ASSERT_MSG(false, "Invalid current state"); return RResult::Fail;
        }
    }

    int64_t JSONBuilder::tokenToInt(const Token& token, int radix)
    {
        ASSERT(token.type == Token::Type::IntegerLiteral);

        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.End());
        const auto result = std::strtol(token.stringSlice.Begin(), &end, radix);

        if (errno == ERANGE)
            getSink().Diagnose(token, Diagnostics::integerLiteralOutOfRange, token.GetContentString(), "int64_t");

        if (end == token.stringSlice.End())
            return result;

        getSink().Diagnose(token, Diagnostics::integerLiteralInvalidBase, token.GetContentString(), radix);
        return 0;
    }

    double JSONBuilder::tokenToFloat(const Token& token)
    {
        ASSERT(token.type == Token::Type::FloatingPointLiteral);

        errno = 0;

        auto end = const_cast<U8Char*>(token.stringSlice.End());
        const auto result = std::strtod(token.stringSlice.Begin(), &end);

        if (errno == ERANGE)
            getSink().Diagnose(token, Diagnostics::floatLiteralOutOfRange, token.GetContentString(), "double");

        if (end == token.stringSlice.End())
            return result;

        getSink().Diagnose(token, Diagnostics::floatLiteralUnexpected, token.GetContentString());
        return 0;
    }
}