#pragma once

#include "common/LinearAllocator.hpp"

namespace RR::Rfx
{
    class ASTBuilder
    {
    public:
        /// Create AST types
        template <typename T>
        T* Create()
        {
            auto alloced = allocator_.Allocate(sizeof(T));
            memset(alloced, 0, sizeof(T));
            return initAndAdd(new (alloced) T);
        }

    private:
        template <typename T>
        T* initAndAdd(T* node)
        {
            /* static_assert(IsValidType<T>::Value);

             node->init(T::kType, this);
             // Only add it if it has a dtor that does some work
             if (!std::is_trivially_destructible<T>::value)
             {
                 // Keep such that dtor can be run on ASTBuilder being dtored
                 m_dtorNodes.add(node);
             }*/
            return node;
        }

    private:
        Common::LinearAllocator allocator_;
    };
}