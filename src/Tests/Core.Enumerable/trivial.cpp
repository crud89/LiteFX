#include "common.h"

#include <array>

enum class Cases : int {
    A = 1,
    B = 2,
    C = 3
};

int main(int /*argc*/, char* /*argv*/[])
{
    auto tests = std::array { Cases::A, Cases::B, Cases::C };
    Enumerable<Cases> testEnm { tests };

    for (int i = 1; auto test : testEnm)
        if (i++ != static_cast<int>(test))
            return -1;

    for (int i = 1; int test : testEnm | std::views::transform([](auto val) { return static_cast<int>(val); }))
        if (i++ != test)
            return -2;

    return 0;
}