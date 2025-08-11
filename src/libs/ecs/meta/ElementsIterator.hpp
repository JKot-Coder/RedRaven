#pragma once

#include "Any.hpp"

namespace RR::Ecs::Meta
{
    struct ElementIterator
    {
        using iterator_category = eastl::random_access_iterator_tag;
        using value_type = Any;
        using difference_type = std::ptrdiff_t;
        using pointer = Any*;
        using reference = Any&;

        ElementIterator() = default;
        ElementIterator(void* data, const ElementInfo* info) : data(data), info(info)
        {
            ASSERT(data);
            ASSERT(info);
        }
        ElementIterator& operator++()
        {
            ++info;
            return *this;
        }
        value_type operator*() const { return Any(static_cast<std::byte*>(data) + info->offset, *info->componentInfo); }

        bool operator==(const ElementIterator& other) const { return info == other.info && data == other.data; }
        bool operator!=(const ElementIterator& other) const { return info != other.info || data != other.data; }

        difference_type operator-(const ElementIterator& other) const
        {
            ASSERT(data == other.data);
            return info - other.info;
        }

    private:
        void* data = nullptr;
        const ElementInfo* info = nullptr;
    };

    struct ElementsSpan
    {
        ElementsSpan() = default;
        ElementsSpan(ElementIterator begin, ElementIterator end) : begin_(begin), end_(end) { }
        ElementIterator begin() const { return begin_; }
        ElementIterator end() const { return end_; }
        size_t size() const { return eastl::distance(begin_, end_); }

    private:
        ElementIterator begin_;
        ElementIterator end_;
    };
}