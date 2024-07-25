#include <cstdint>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    UNUSED(name, flags, debugFlags, file, line);
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    UNUSED(alignment, alignmentOffset, pName, flags, debugFlags, file, line);
    return new uint8_t[size];
}