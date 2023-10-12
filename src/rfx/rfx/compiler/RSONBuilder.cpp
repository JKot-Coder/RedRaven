#include "RSONBuilder.hpp"

#include "DiagnosticCore.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/Token.hpp"
#include "rfx/core/SourceLocation.hpp"

namespace RR::Rfx
{
    RSONBuilder::RSONBuilder(const std::shared_ptr<CompileContext>& context) : expect_(Expect::ObjectKey),
                                                                               context_(context)
    {
        root_ = RSONValue::MakeEmptyObject();
        stack_.emplace(root_);
    }

    DiagnosticSink& RSONBuilder::getSink() const { return context_->sink; }

    RResult RSONBuilder::checkNoInheritanceAlloved(RSONValue::Type value_type)
    {
        if (parents_.empty())
            return RResult::Ok;

        getSink().Diagnose(parents_.front().first, Diagnostics::invalidTypeForInheritance, key_, value_type);
        return RResult::Fail;
    }

    RResult RSONBuilder::StartObject()
    {
        auto value = RSONValue::MakeEmptyObject();
        RR_RETURN_ON_FAIL(add(value));
        stack_.emplace(parents_, value);
        expect_ = Expect::ObjectKey;
        parents_.clear();
        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndObject()
    {
        ASSERT(currentValue().type == RSONValue::Type::Object);
        ASSERT(expect_ == Expect::ObjectKey);

        // Inheritance
        size_t size = 0;
        for (const auto parent : currentContext().parents)
            size += parent.second->size();

        currentValue().container->reserve(size);

        for (const auto parent : currentContext().parents)
            currentValue().container->insert(parent.second->begin(), parent.second->end());

        RSONValue result = currentValue();
        stack_.pop();
        key_.Reset();
        parents_.clear();
        expect_ = currentValue().type == RSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;

        return result;
    }

    RResult RSONBuilder::StartArray()
    {
        auto value = RSONValue::MakeEmptyArray();

        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(value.type));
        RR_RETURN_ON_FAIL(add(value));
        stack_.emplace(value);
        expect_ = Expect::ArrayValue;

        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndArray()
    {
        ASSERT(currentValue().type == RSONValue::Type::Array);
        ASSERT(expect_ == Expect::ArrayValue);

        RSONValue result = currentValue();
        stack_.pop();
        key_.Reset();

        expect_ = currentValue().type == RSONValue::Type::Array ? Expect::ArrayValue : Expect::ObjectKey;
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

    RResult RSONBuilder::AddKey(const Token& key)
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

    RResult RSONBuilder::AddValue(RSONValue value)
    {
        ASSERT(value.type != RSONValue::Type::Invalid);
        RR_RETURN_ON_FAIL(checkNoInheritanceAlloved(value.type));
        return add(std::move(value));
    }

    RResult RSONBuilder::add(RSONValue value)
    {
        switch (expect_)
        {
            case Expect::ArrayValue: currentValue().append(std::move(value)); break;
            case Expect::ObjectValue:
                currentValue().emplace(key_, std::move(value));
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