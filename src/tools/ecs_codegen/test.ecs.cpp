#include "test.hpp"

#include "trash.hpp"
[[clang::annotate("qwe")]]
struct [[clang::annotate("zxc")]]   __attribute__((annotate("GEN_RTTR"), annotate("GEN_RTTR<float>"))) Test {
    int a;
    float b;

    struct Test2 {
        int c;
        float d;
    } a2;
    QWE z;

    void (*trash[2])() = {[](){}, [](){}};//, d;

};



void test() {
}