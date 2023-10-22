#include "RSONBuilder.hpp"

#include "DiagnosticCore.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/Token.hpp"
#include "rfx/core/SourceLocation.hpp"

namespace RR::Rfx
{
    RSONBuilder::RSONBuilder(const std::shared_ptr<CompileContext>& context) : expect_(Expect::ObjectValue),
                                                                               context_(context)
    {
        root_ = RSONValue::MakeEmptyObject();
        stack_.emplace(root_);
    }

    DiagnosticSink& RSONBuilder::getSink() const { return context_->sink; }

    RResult RSONBuilder::checkNoInheritanceAlloved(const Token& token, RSONValue::Type value_type)
    {
        if (parents_.empty())
            return RResult::Ok;
        std::ignore = value_type;

        getSink().Diagnose(parents_.front().first, Diagnostics::invalidTypeForInheritance, token.stringSlice, value_type);
        return RResult::Fail;
    }

    RResult RSONBuilder::StartObject()
    {
        auto value = RSONValue::MakeEmptyObject();
        stack_.emplace(parents_, value);
        expect_ = Expect::ObjectValue;
        parents_.clear();
        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndObject()
    {
        ASSERT(currentValue().type == RSONValue::Type::Object);
        ASSERT(expect_ == Expect::ObjectValue);

        // Inheritance
        size_t size = 0;
        for (const auto parent : currentContext().parents)
            size += parent.second->size();

        currentValue().container->reserve(size);

        for (const auto parent : currentContext().parents)
            currentValue().container->insert(parent.second->begin(), parent.second->end());

        RSONValue result = currentValue();
        stack_.pop();
        parents_.clear();
        expect_ = currentValue().type == RSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectValue;

        return result;
    }

    RResult RSONBuilder::StartArray()
    {
        auto value = RSONValue::MakeEmptyArray();

        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved({}, value.type));

        stack_.emplace(value);
        parents_.clear();
        expect_ = Expect::ArrayValue;

        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndArray()
    {
        ASSERT(currentValue().type == RSONValue::Type::Array);
        ASSERT(expect_ == Expect::ArrayValue);

        RSONValue result = currentValue();
        stack_.pop();

        expect_ = currentValue().type == RSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectValue;
        return result;
    }

    void RSONBuilder::StartInrehitance()
    {
        ASSERT(expect_ == (currentValue().type == RSONValue::Type::Object ? Expect::ObjectValue : Expect::ArrayValue));
        ASSERT(parents_.size() == 0);
        expect_ = Expect::Parent;
    }

    void RSONBuilder::EndInrehitance()
    {
        ASSERT(expect_ == Expect::Parent);
        ASSERT(parents_.size() != 0);
        expect_ = currentValue().type == RSONValue::Type::Object ? Expect::ObjectValue : Expect::ArrayValue;
    }

    RResult RSONBuilder::AddParent(const Token& parent)
    {
        ASSERT(expect_ == Expect::Parent);
        ASSERT(parent.type == Token::Type::StringLiteral || parent.type == Token::Type::Identifier);
        const auto parentName = parent.stringSlice;

        auto value = root_.Find(parentName);
        switch (value.type)
        {
            case RSONValue::Type::Object:
            {
                parents_.emplace_back(parent, value.container.get());
                break;
            }
            case RSONValue::Type::Invalid:
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

    RResult RSONBuilder::AddKeyValue(const Token& key, RSONValue value)
    {
        ASSERT(expect_ == Expect::ObjectValue);
        ASSERT(key.type == Token::Type::StringLiteral || key.type == Token::Type::Identifier);
        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(key, value.type));
        const auto keyName = key.stringSlice;

        if (currentValue().Contains(keyName))
        {
            getSink().Diagnose(key, Diagnostics::duplicateKey, keyName);
            return RResult::AlreadyExist;
        }

        currentValue().emplace(keyName, std::move(value));
        return RResult::Ok;
    }

    RResult RSONBuilder::AddValue(const Token& valuet, RSONValue value)
    {
        ASSERT(value.type != RSONValue::Type::Invalid);
        ASSERT(expect_ == Expect::ArrayValue);
        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(valuet, value.type));
        currentValue().append(std::move(value));
        return RResult::Ok;
    }
}