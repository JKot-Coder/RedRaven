#include "ecs/meta/TypeTraits.hpp"
#include "ecs/meta/ComponentTraits.hpp"
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
    int x = 0;
};

class Class0
{
};

static_assert(RR::Ecs::Meta::TypeName<Test::Foo>::string_view() == "Test::Foo");
static_assert(RR::Ecs::Meta::TypeName<Test::Foo::Bar>::string_view() == "Test::Foo::Bar");
static_assert(RR::Ecs::Meta::TypeName<Struct0>::string_view() == "Struct0");
static_assert(RR::Ecs::Meta::TypeName<Class0>::string_view() == "Class0");
static_assert(RR::Ecs::Meta::TypeName<TemplatedFoo<int>>::string_view() == "TemplatedFoo<int>");
static_assert(RR::Ecs::Meta::TypeName<TemplatedFoo<Test::Foo::Bar>>::string_view() == "TemplatedFoo<Test::Foo::Bar>");
static_assert(RR::Ecs::Meta::TypeName<int>::string_view() == "int");
static_assert(RR::Ecs::Meta::TypeName<float>::string_view() == "float");

static_assert(RR::Ecs::Meta::IsTag<Struct0> == false);
static_assert(RR::Ecs::Meta::IsTag<Class0> == true);
static_assert(RR::Ecs::Meta::IsTag<TemplatedFoo<int>> == true);