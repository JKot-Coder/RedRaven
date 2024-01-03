#include "RSONBuilder.hpp"

#include "DiagnosticCore.hpp"
#include "common/Result.hpp"
#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/Token.hpp"
#include "rfx/core/SourceLocation.hpp"

namespace RR::Rfx
{
    RSONBuilder::RSONBuilder(const std::shared_ptr<CompileContext>& context) : context_(context)
    {
        root_ = RSONValue::MakeEmptyObject();
        stack_.emplace_back(root_);
    }

    DiagnosticSink& RSONBuilder::getSink() const { return context_->sink; }

    RResult RSONBuilder::StartObject()
    {
        stack_.emplace_back(RSONValue::MakeEmptyObject());
        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndObject()
    {
        ASSERT(currentValue().type == RSONValue::Type::Object);

        // Inheritance
        size_t size = 0;
        for (const auto parent : currentContext().parents)
            size += parent->size();

        currentValue().container->reserve(size);

        for (const auto parent : currentContext().parents)
            currentValue().container->insert(parent->begin(), parent->end());

        RSONValue result = currentValue();
        stack_.pop_back();

        return result;
    }

    RResult RSONBuilder::StartArray()
    {
        stack_.emplace_back(RSONValue::MakeEmptyArray());
        return RResult::Ok;
    }

    RSONValue RSONBuilder::EndArray()
    {
        ASSERT(currentValue().type == RSONValue::Type::Array);

        RSONValue result = currentValue();
        stack_.pop_back();

        return result;
    }

    RResult RSONBuilder::Inheritance(const Token& initiatingToken, const RSONValue& parents)
    {
        ASSERT(currentValue().type == RSONValue::Type::Object);

        const auto inherite = [this, &initiatingToken](const RSONValue& refValue)
        {
            ASSERT(refValue.type == RSONValue::Type::Reference);
            ASSERT(refValue.referenceValue.reference);
            const auto& parent = *refValue.referenceValue.reference;

            switch (parent.type)
            {
                case RSONValue::Type::Object:
                {
                    currentContext().parents.emplace_back(parent.container.get());
                    break;
                }
                case RSONValue::Type::Invalid:
                {
                    ASSERT_MSG(false, "Unreacheable");
                    getSink().Diagnose(initiatingToken, Diagnostics::undeclaredIdentifier, refValue.referenceValue.path);
                    return RResult::NotFound;
                }
                default:
                {
                    getSink().Diagnose(initiatingToken, Diagnostics::invalidParentType, refValue.referenceValue.path, parent.type);
                    return RResult::Fail;
                }
            }
            return RResult::Ok;
        };

        if (parents.type == RSONValue::Type::Reference)
            return inherite(parents);

        if (!parents.IsArray())
        {
            getSink().Diagnose(initiatingToken, Diagnostics::invalidParentsValue, RSONValueTypeToString(parents.type));
            return RResult::Fail;
        }

        for (auto parent : parents)
        {
            if (parent.second.type != RSONValue::Type::Reference)
            {
                getSink().Diagnose(initiatingToken, Diagnostics::invalidParentIndetifier, RSONValueTypeToString(parent.second.type));
                return RResult::Fail;
            }

            RR_RETURN_ON_FAIL(inherite(parent.second));
        }

        return RResult::Ok;
    }

    RResult RSONBuilder::AddKeyValue(const Token& key, RSONValue value)
    {
        ASSERT(currentValue().type == RSONValue::Type::Object);
        ASSERT(key.type == Token::Type::StringLiteral || key.type == Token::Type::Identifier);
        const auto keyName = key.stringSlice;

        if (currentValue().Contains(keyName))
        {
            getSink().Diagnose(key, Diagnostics::duplicateKey, keyName);
            return RResult::AlreadyExist;
        }

        currentValue().emplace(keyName, std::move(value));
        return RResult::Ok;
    }

    RResult RSONBuilder::AddValue(RSONValue value)
    {
        ASSERT(value.type != RSONValue::Type::Invalid);
        ASSERT(currentValue().type == RSONValue::Type::Array);

        currentValue().append(std::move(value));
        return RResult::Ok;
    }

    RResult RSONBuilder::ResolveReference(RSONValue& value)
    {
        ASSERT(value.type == RSONValue::Type::Reference);

        const auto split = StringSplit<UnownedStringSlice, '.'>(value.referenceValue.path);

        for (auto it = stack_.rbegin() + 1; it != stack_.rend(); it++)
        {
            const RSONValue& refVal = it->value.Find(split);

            if (refVal.type != RSONValue::Type::Invalid)
            {
                value.referenceValue.reference = &refVal;
                return RResult::Ok;
            }
        }

        return RResult::NotFound;
    }
}