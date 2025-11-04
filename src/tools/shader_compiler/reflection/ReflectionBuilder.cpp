#include "ReflectionBuilder.hpp"

#include "ReflectionResourceType.hpp"
#include "effect_library/ShaderReflectionData.hpp"

#include "Utils.hpp"

#include "slang.h"
using namespace slang;

#include <iostream>

namespace RR
{
    /* ----------------  Metdods from Falcor ---------------- */

    // Represents one link in a "breadcrumb trail" leading from a particular variable
    // back through the path of member-access operations that led to it.
    // E.g., when trying to construct information for `foo.bar.baz`
    // we might have a path that consists of:
    //
    // - An link for the field `baz` in type `Bar` (which knows its offset)
    // - An link for the field `bar` in type `Foo`
    // - An link for the top-level shader parameter `foo`
    //
    // To compute the correct offset for `baz` we can walk up this chain
    // and add up offsets.
    //
    // In simple cases, one can track this info top-down, by simply keeping
    // a "running total" offset, but that doesn't account for the fact that
    // `baz` might be a texture, UAV, sampler, or uniform, and the offset
    // we'd need to track for each case is different.
    //
    struct ReflectionPathLink
    {
        const ReflectionPathLink* pParent = nullptr;
        VariableLayoutReflection* pVar = nullptr;
    };

    // Represents a full"breadcrumb trail" leading from a particular variable
    // back through the path of member-access operations that led to it.
    //
    // The `pPrimary` field represents the main path that gets used for
    // ordinary uniform, texture, buffer, etc. variables. In the 99% case
    // this is all that ever gets used.
    //
    // The `pDeferred` field represents a secondary path that describes
    // where the data that arose due to specialization ended up.
    //
    // E.g., if we have a program like:
    //
    //     struct MyStuff { IFoo f; Texture2D t; }
    //     MyStuff gStuff;
    //     Texture2D gOther;
    //
    // Then `gStuff` will be assigned a starting `t` register of `t0`,
    // and the *primary* path for `gStuff.t` will show that offset.
    //
    // However, if `gStuff.f` gets specialized to some type `Bar`:
    //
    //     struct Bar { Texture2D b; }
    //
    // Then the `gStuff.f.b` field also needs a texture register to be
    // assigned. It can't use registers `t0` or `t1` since those were
    // already allocated in the unspecialized program (to `gStuff.t`
    // and `gOther`, respectively), so it needs to use `t2`.
    //
    // But that means that the allocation for `gStuff` is split into two
    // pieces: a "primary" allocation for `gStuff.t`, and then a secondary
    // allocation for `gStuff.f` that got "deferred" until after specialization
    // (which means it comes after all the un-specialized parameters).
    //
    // The Slang reflection information lets us query both the primary
    // and deferred allocation/layout for a shader parameter, and we
    // need to handle both in order to support specialization.
    //
    struct ReflectionPath
    {
        ReflectionPathLink* pPrimary = nullptr;
        ReflectionPathLink* pDeferred = nullptr;
    };

    // A helper RAII type to extend a `ReflectionPath` with
    // additional links as needed based on the reflection
    // information from a Slang `VariableLayoutReflection`.
    //
    struct ExtendedReflectionPath : ReflectionPath
    {
        ExtendedReflectionPath(ReflectionPath const* pParent, VariableLayoutReflection* pVar)
        {
            // If there is any path stored in `pParent`,
            // then that will be our starting point.
            //
            if (pParent)
            {
                pPrimary = pParent->pPrimary;
                pDeferred = pParent->pDeferred;
            }

            // Next, if `pVar` has a primary layout (and/or
            // an optional pending/deferred layout), then
            // we will extend the appropriate breadcrumb
            // trail with its information.
            //
            if (pVar)
            {
                primaryLinkStorage.pParent = pPrimary;
                primaryLinkStorage.pVar = pVar;
                pPrimary = &primaryLinkStorage;

                if (auto pDeferredVar = pVar->getPendingDataLayout())
                {
                    deferredLinkStorage.pParent = pDeferred;
                    deferredLinkStorage.pVar = pDeferredVar;
                    pDeferred = &deferredLinkStorage;
                }
            }
        }

        // These "storage" fields are used in the constructor
        // when it needs to allocate additional links. By pre-allocating
        // them here in the body of the type we avoid having to do
        // heap allocation when constructing an extended path.
        //
        ReflectionPathLink primaryLinkStorage;
        ReflectionPathLink deferredLinkStorage;
    };

    // Determine if a Slang type layout consumes any storage/resources of the given kind
    static bool hasUsage(TypeLayoutReflection* pSlangTypeLayout, SlangParameterCategory resourceKind)
    {
        auto kindCount = pSlangTypeLayout->getCategoryCount();
        for (unsigned int ii = 0; ii < kindCount; ++ii)
        {
            if (SlangParameterCategory(pSlangTypeLayout->getCategoryByIndex(ii)) == resourceKind)
                return true;
        }
        return false;
    }

    // Given a "breadcrumb trail" (reflection path), determine
    // the actual register/binding that will be used by a leaf
    // parameter for the given resource kind.
    static size_t getRegisterIndexFromPath(const ReflectionPathLink* pPath, SlangParameterCategory category)
    {
        uint32_t offset = 0;
        for (auto pp = pPath; pp; pp = pp->pParent)
        {
            if (pp->pVar)
            {
                // We are in the process of walking up from a leaf
                // shader variable to the root (some global shader
                // parameter).
                //
                // If along the way we run into a parameter block,
                // *and* that parameter block has been allocated
                // into its own register space, then we should stop
                // adding contributions to the register/binding of
                // the leaf parameter, since any register offsets
                // coming from "above" this point shouldn't affect
                // the register/binding of a parameter inside of
                // the parameter block.
                //
                // TODO: This logic is really fiddly and doesn't
                // seem like something Falcor should have to do.
                // The Slang library should be provided utility
                // functions to handle this stuff.
                //
                if (pp->pVar->getType()->getKind() == TypeReflection::Kind::ParameterBlock &&
                    hasUsage(pp->pVar->getTypeLayout(), SLANG_PARAMETER_CATEGORY_REGISTER_SPACE) &&
                    category != SLANG_PARAMETER_CATEGORY_REGISTER_SPACE)
                {
                    return offset;
                }

                offset += (uint32_t)pp->pVar->getOffset(category);
                continue;
            }
            THROW("Invalid reflection path");
        }
        return offset;
    }

    static uint32_t getRegisterSpaceFromPath(const ReflectionPathLink* pPath, SlangParameterCategory category)
    {
        uint32_t offset = 0;
        for (auto pp = pPath; pp; pp = pp->pParent)
        {
            if (pp->pVar)
            {
                // Similar to the case above in `getRegisterIndexFromPath`,
                // if we are walking from a member in a parameter block
                // up to the block itself, then the space for our parameter
                // should be offset by the register space assigned to
                // the block itself, and we should stop walking up
                // the breadcrumb trail.
                //
                // TODO: Just as in `getRegisterIndexFromPath` this is way
                // too subtle, and Slang should be providing a service
                // to compute this.
                //
                if (pp->pVar->getTypeLayout()->getKind() == slang::TypeReflection::Kind::ParameterBlock)
                {
                    return offset + (uint32_t)getRegisterIndexFromPath(pp, SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);
                }
                offset += (uint32_t)pp->pVar->getBindingSpace(category);
                continue;
            }

            THROW("Invalid reflection path");
        }
        return offset;
    }

    static ReflectionResourceType::Type getResourceType(TypeReflection* pSlangType)
    {
        switch (pSlangType->unwrapArray()->getKind())
        {
            case TypeReflection::Kind::ParameterBlock:
            case TypeReflection::Kind::ConstantBuffer:
                return ReflectionResourceType::Type::ConstantBuffer;
            case TypeReflection::Kind::SamplerState: return ReflectionResourceType::Type::Sampler;
            case TypeReflection::Kind::ShaderStorageBuffer: return ReflectionResourceType::Type::StructuredBuffer;
            case TypeReflection::Kind::TextureBuffer: return ReflectionResourceType::Type::TypedBuffer;
            case TypeReflection::Kind::Resource:
                switch (pSlangType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK)
                {
                    case SLANG_STRUCTURED_BUFFER: return ReflectionResourceType::Type::StructuredBuffer;
                    case SLANG_BYTE_ADDRESS_BUFFER: return ReflectionResourceType::Type::RawBuffer;
                    case SLANG_TEXTURE_BUFFER: return ReflectionResourceType::Type::TypedBuffer;
                    case SLANG_ACCELERATION_STRUCTURE: return ReflectionResourceType::Type::AccelerationStructure;
                    case SLANG_TEXTURE_1D:
                    case SLANG_TEXTURE_2D:
                    case SLANG_TEXTURE_3D:
                    case SLANG_TEXTURE_CUBE:
                        return ReflectionResourceType::Type::Texture;
                    default:
                        UNREACHABLE();
                        return ReflectionResourceType::Type(-1);
                }
            default:
                UNREACHABLE();
                return ReflectionResourceType::Type(-1);
        }
    }

    static ReflectionResourceType::Dimensions getResourceDimensions(SlangResourceShape shape)
    {
        switch (shape)
        {
            case SLANG_TEXTURE_1D: return ReflectionResourceType::Dimensions::Texture1D;
            case SLANG_TEXTURE_1D_ARRAY: return ReflectionResourceType::Dimensions::Texture1DArray;
            case SLANG_TEXTURE_2D: return ReflectionResourceType::Dimensions::Texture2D;
            case SLANG_TEXTURE_2D_ARRAY: return ReflectionResourceType::Dimensions::Texture2DArray;
            case SLANG_TEXTURE_2D_MULTISAMPLE: return ReflectionResourceType::Dimensions::Texture2DMS;
            case SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY: return ReflectionResourceType::Dimensions::Texture2DMSArray;
            case SLANG_TEXTURE_3D: return ReflectionResourceType::Dimensions::Texture3D;
            case SLANG_TEXTURE_CUBE: return ReflectionResourceType::Dimensions::TextureCube;
            case SLANG_TEXTURE_CUBE_ARRAY: return ReflectionResourceType::Dimensions::TextureCubeArray;
            case SLANG_ACCELERATION_STRUCTURE: return ReflectionResourceType::Dimensions::AccelerationStructure;

            case SLANG_TEXTURE_BUFFER:
            case SLANG_STRUCTURED_BUFFER:
            case SLANG_BYTE_ADDRESS_BUFFER:
                return ReflectionResourceType::Dimensions::Buffer;

            default: return ReflectionResourceType::Dimensions::Unknown;
        }
    }

    static ParameterCategory getParameterCategory(TypeLayoutReflection* pTypeLayout)
    {
        ParameterCategory category = pTypeLayout->getParameterCategory();
        if (category == ParameterCategory::Mixed)
        {
            switch (pTypeLayout->getKind())
            {
                case TypeReflection::Kind::ConstantBuffer:
                case TypeReflection::Kind::ParameterBlock:
                case TypeReflection::Kind::None:
                    category = ParameterCategory::ConstantBuffer;
                    break;
                default:
                    UNREACHABLE();
                    return ParameterCategory::None;
            }
        }
        return category;
    }

    /*   static ParameterCategory getParameterCategory(VariableLayoutReflection* pVarLayout)
       {
           return getParameterCategory(pVarLayout->getTypeLayout());
       }*/
}

namespace RR
{
    void reflectResourceType(TypeLayoutReflection* pSlangType, ReflectionPath* pPath)
    {
        ASSERT(pPath->pPrimary && pPath->pPrimary->pVar);

        ReflectionResourceType::Type type = getResourceType(pSlangType->getType());
        ReflectionResourceType::Dimensions dims = getResourceDimensions(pSlangType->getResourceShape());

        ParameterCategory category = getParameterCategory(pSlangType);

        auto regIndex = (uint32_t)getRegisterIndexFromPath(pPath->pPrimary, SlangParameterCategory(category));
        auto regSpace = getRegisterSpaceFromPath(pPath->pPrimary, SlangParameterCategory(category));

        std::cout << " " << enumToString(type) << " regIndex: " << regIndex << " regSpace: " << regSpace << " dims: " << enumToString(dims) << std::endl;

        switch (type)
        {
            default:
                break;

            case ReflectionResourceType::Type::ConstantBuffer:
            {
                break;
            }
        }
    }

    void ReflectionBuilder::Build(ShaderReflection* reflection)
    {
        ASSERT(reflection);

        auto slangGlobalParamsTypeLayout = reflection->getGlobalParamsTypeLayout();

        // The Slang type layout for the global scope either directly represents the
        // parameters as a struct type `G`, or it represents those parameters wrapped
        // up into a constant buffer like `ConstantBuffer<G>`. If we are in the latter
        // case, then we want to get the element type (`G`) from the constant buffer
        // type layout.
        //
        if (auto elementTypeLayout = slangGlobalParamsTypeLayout->getElementTypeLayout())
            slangGlobalParamsTypeLayout = elementTypeLayout;

        for (uint32_t i = 0; i < slangGlobalParamsTypeLayout->getFieldCount(); i++)
        {

            auto field = slangGlobalParamsTypeLayout->getFieldByIndex(i);
            ExtendedReflectionPath path(nullptr, field);

            auto pSlangType = field->getTypeLayout();
            auto kind = pSlangType->getType()->getKind();
            switch (kind)
            {
                case TypeReflection::Kind::ParameterBlock:
                case TypeReflection::Kind::ConstantBuffer:
                    std::cout << field->getName();
                    reflectResourceType(pSlangType, &path);
                    break;

                case TypeReflection::Kind::Scalar:
                case TypeReflection::Kind::Matrix:
                case TypeReflection::Kind::Vector:
                    break;
                default:
                    ASSERT_MSG(false, "unknown type"); // maybe throw
            }
        }

        //  slangGlobalParamsTypeLayout->
    }
}