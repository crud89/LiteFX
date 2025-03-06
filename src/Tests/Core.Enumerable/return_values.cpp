#include "common.h"

Enumerable<Base&> foos_mutable_vec(std::vector<Foo>& bars) {
    return bars;
}

Enumerable<const Base&> const_foos_mutable_vec(std::vector<Foo>& bars) {
    return bars;
}

Enumerable<const Base&> const_foos_const_vec(const std::vector<Foo>& bars) {
    return bars;
}

Enumerable<Base&> const_foos_from_filtered_vec(std::vector<Foo>& bars) {
    return bars | std::views::drop(1) | std::views::take(2);
}

Enumerable<Base&> foos_from_rvalue_bars(std::vector<Foo>&& bars) {
    return std::move(bars);
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::vector<Foo> bars;
    //bars.reserve(4);
    bars.emplace_back(1);
    bars.emplace_back(2);
    bars.emplace_back(3);
    bars.emplace_back(4);

    for (int i{ 1 }; auto & foo : foos_mutable_vec(bars))
        if (foo.index() != i++)
            return -1;

    for (int i{ 1 }; auto& foo : const_foos_mutable_vec(bars))
        if (foo.index() != i++)
            return -2;

    for (int i{ 1 }; auto& foo : const_foos_const_vec(bars))
        if (foo.index() != i++)
            return -3;

    for (int i{ 2 }; auto& foo : const_foos_from_filtered_vec(bars))
        if (foo.index() != i++)
            return -4;

    for (int i{ 1 }; auto& foo : foos_from_rvalue_bars(std::move(bars)))
        if (foo.index() != i++)
            return -5;

    return 0;
}