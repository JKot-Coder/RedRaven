#include "Math.hpp"

namespace RR
{
    namespace Common
    {
        void test()
        {
            Vector2 cc(1);
            Vector2 bb(2, 2);

            Vector2 asd = cc + bb;
            Vector2 norm = asd.Normal();
            Vector2::ZERO;

            Radian pi = Radian(PI);
            Degree c = pi;
            c += 60.0f;
            pi = c;
            pi = pi - PI;
            pi = pi.ToDegree();
            c = pi.ToDegree();
        }
    }
}