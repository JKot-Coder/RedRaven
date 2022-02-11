#pragma once

// TODO IS THIS USED?

namespace RR
{
    namespace Rfx
    {
        class IInputStream
        {
        public:
            IInputStream() = default;
            virtual ~IInputStream() = default;

            virtual U8String ReadLine() = 0;
            virtual bool HasNextLine() const = 0;
        };

        class StringInputStream : public IInputStream
        {
        public:
            StringInputStream() = delete;
            StringInputStream(const U8String& source);

            StringInputStream(const StringInputStream& inputStream);
            StringInputStream(StringInputStream&& inputStream);
            virtual ~StringInputStream() = default;

            U8String ReadLine() override;
            bool HasNextLine() const override;

        private:
            U8String sourceString_;
        };
    }
}