#include <boost/ut.hpp>

namespace ut = boost::ut;

namespace cfg2
{
    struct printer : ut::printer
    {
        template <class T>
        auto& operator<<(T&& t)
        {
            std::cerr << std::forward<T>(t);
            return *this;
        }
    };
} // namespace cfg

template <>
auto ut::cfg<ut::override> = ut::runner<ut::reporter<cfg2::printer>> {};

#ifdef OS_WINDOWS
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

#else
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
#endif
    using namespace ut;
    expect((1 == 2_i)) << "Not enough parameters!";

    return cfg<override>.run();
}

ut::suite basic = [] {
    using namespace ut;
    should("equal") = [] { expect(43_i == 42); };
};