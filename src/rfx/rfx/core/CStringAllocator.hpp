#pragma once

namespace RR::Rfx
{
    template <typename T>
    class CStringAllocator final
    {
    public:
        ~CStringAllocator()
        {
            for (const auto cstring : cstring_)
                delete[] cstring;
        }

        void Allocate(const std::vector<std::basic_string<T>>& strings)
        {
            for (const auto& string : strings)
                Allocate(string);
        }

        T* Allocate(const std::basic_string<T>& string)
        {
            const auto stringLength = string.length();

            auto cString = new T[stringLength + 1];
            string.copy(cString, stringLength);
            cString[stringLength] = '\0';

            cstring_.push_back(cString);
            return cString;
        }

        const std::vector<T*>& GetCStrings() const { return cstring_; }

    private:
        std::vector<T*> cstring_;
    };
}