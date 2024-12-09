#include "TypeName.hpp"

namespace Test
{
    struct Foo
    {
        class Bar
        {
        };
    };
}

template <typename T>
class TemplatedFoo
{
};

struct Struct0
{
};

class Class0
{
};

// Sanity checks
static_assert(RR::Ecs::TypeName<Test::Foo>::string_view() == "Test::Foo");
static_assert(RR::Ecs::TypeName<Test::Foo::Bar>::string_view() == "Test::Foo::Bar");
static_assert(RR::Ecs::TypeName<Struct0>::string_view() == "Struct0");
static_assert(RR::Ecs::TypeName<Class0>::string_view() == "Class0");
static_assert(RR::Ecs::TypeName<TemplatedFoo<int>>::string_view() == "TemplatedFoo<int>");
static_assert(RR::Ecs::TypeName<TemplatedFoo<Test::Foo::Bar>>::string_view() == "TemplatedFoo<Test::Foo::Bar>");
static_assert(RR::Ecs::TypeName<int>::string_view() == "int");
static_assert(RR::Ecs::TypeName<float>::string_view() == "float");