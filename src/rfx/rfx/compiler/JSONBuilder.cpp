#include "JSONBuilder.hpp"

#include "DiagnosticCore.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/Token.hpp"
#include "rfx/core/SourceLocation.hpp"

namespace RR::Rfx
{
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

    JSONBuilder::JSONBuilder(const std::shared_ptr<CompileContext>& context) : expect_(Expect::ObjectKey),
                                                                               context_(context)
    {
        root_ = JSONValue::MakeEmptyObject();
        stack_.emplace(root_);
    }

    DiagnosticSink& JSONBuilder::getSink() const { return context_->sink; }

    RResult JSONBuilder::checkNoInheritanceAlloved(JSONValue::Type value_type)
    {
        if (parents_.empty())
            return RResult::Ok;

        getSink().Diagnose(parents_.front().first, Diagnostics::invalidTypeForInheritance, key_, value_type);
        return RResult::Fail;
    }

    RResult JSONBuilder::StartObject()
    {
        auto value = JSONValue::MakeEmptyObject();
        RR_RETURN_ON_FAIL(add(value));
        stack_.emplace(parents_, value);
        expect_ = Expect::ObjectKey;
        parents_.clear();
        return RResult::Ok;
    }

    void JSONBuilder::EndObject()
    {
        ASSERT(currentValue().type == JSONValue::Type::Object);
        ASSERT(expect_ == Expect::ObjectKey);

        // Inheritance
        size_t size = 0;
        for (const auto parent : currentContext().parents)
            size += parent.second->size();

        currentValue().container->reserve(size);

        for (const auto parent : currentContext().parents)
            currentValue().container->insert(parent.second->begin(), parent.second->end());

        stack_.pop();
        key_.Reset();
        parents_.clear();
        expect_ = currentValue().type == JSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;
    }

    RResult JSONBuilder::StartArray()
    {
        auto value = JSONValue::MakeEmptyArray();

        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(value.type));
        RR_RETURN_ON_FAIL(add(value));
        stack_.emplace(value);
        expect_ = Expect::ArrayValue;

        return RResult::Ok;
    }

    void JSONBuilder::EndArray()
    {
        ASSERT(currentValue().type == JSONValue::Type::Array);
        ASSERT(expect_ == Expect::ArrayValue);
        stack_.pop();
        key_.Reset();

        expect_ = currentValue().type == JSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;
    }

    void JSONBuilder::StartInrehitance()
    {
        ASSERT(expect_ == (currentValue().type == JSONValue::Type::Object ? Expect::ObjectValue : Expect::ArrayValue));
        ASSERT(parents_.size() == 0);
        expect_ = Expect::Parent;
    }

    void JSONBuilder::EndInrehitance()
    {
        ASSERT(expect_ == Expect::Parent);
        ASSERT(parents_.size() != 0);
        expect_ = currentValue().type == JSONValue::Type::Object ? Expect::ObjectValue : Expect::ArrayValue;
    }

    RResult JSONBuilder::AddParent(const Token& parent)
    {
        ASSERT(expect_ == Expect::Parent);
        ASSERT(parent.type == Token::Type::StringLiteral || parent.type == Token::Type::Identifier);
        const auto parentName = parent.stringSlice;

        auto value = root_.Find(parentName);
        switch (value.type)
        {
            case JSONValue::Type::Object:
            {
                parents_.emplace_back(parent, value.container.get());
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
        const auto keyName = key.stringSlice;

        if (currentValue().Contains(keyName))
        {
            getSink().Diagnose(key, Diagnostics::duplicateKey, keyName);
            return RResult::AlreadyExist;
        }

        expect_ = Expect::ObjectValue;
        key_ = keyName;

        return RResult::Ok;
    }

    RResult JSONBuilder::AddValue(JSONValue&& value)
    {
        ASSERT(value.type != JSONValue::Type::Invalid);
        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(value.type));
        return add(value);
    }

    RResult JSONBuilder::add(JSONValue&& value)
    {
        switch (expect_)
        {
            case Expect::ArrayValue: currentValue().append(std::move(value)); break;
            case Expect::ObjectValue:
                currentValue()[key_] = value;
                key_.Reset();
                expect_ = Expect::ObjectKey;
                break;
            default:
                ASSERT_MSG(false, "Invalid current state");
                return RResult::Fail;
        }

        return RResult::Ok;
    }
}