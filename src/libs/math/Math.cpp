#include "VectorMath.hpp"

namespace RR
{
    void test()
    {
        Vector2 cc(1);
        Vector2 bb(2, 2);

        Vector2 asd = cc + bb;
        Vector2 norm = asd.Normal();
        (void)norm;

        Radian pi = Radian(PI);
        Degree c = pi;
        c += 60.0f;
        pi = c;
        pi = pi - PI;
        pi = pi.ToDegree();
        c = pi.ToDegree();
    }
}