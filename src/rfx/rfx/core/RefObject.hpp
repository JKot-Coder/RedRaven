#pragma once

#include <atomic>

namespace RR::Rfx
{
    // Base class for all reference-counted objects
    class RefObject
    {
    public:
        RefObject() = default;
        //RefObject(const RefObject&) = default;
        virtual ~RefObject() { }
      //  RefObject& operator=(const RefObject& rhs) = default;

        uint32_t addReference() { return ++referenceCount_; }
        uint32_t decreaseReference() { return --referenceCount_; }
        uint32_t releaseReference()
        {
            ASSERT(referenceCount_ != 0);
            if (--referenceCount_ == 0)
            {
                delete this;
                return 0;
            }
            return referenceCount_;
        }

        bool isUniquelyReferenced() const
        {
            ASSERT(referenceCount_ != 0);
            return referenceCount_ == 1;
        }

        uint32_t debugGetReferenceCount() const
        {
            return referenceCount_;
        }

    private:
        std::atomic<uint32_t> referenceCount_ = 0;
    };
}