#include "common.h"

int main(int /*argc*/, char* /*argv*/[])
{
    std::vector<Bar> bars;
    //bars.reserve(4);
    bars.emplace_back(1);
    bars.emplace_back(2);
    bars.emplace_back(3);
    bars.emplace_back(4);

    Enumerable<const Base&> enm = bars |
        std::views::transform([](const auto& bar) -> const Base& { return bar; }) |
        std::views::filter([](const auto& base) { return base.index() % 2 == 0; });

    try
    {
        std::ranges::for_each(enm, [i = 2](const auto& base) mutable {
            if (i != base.index())
                throw;
            else
                i += 2;
        });
    }
    catch (...)
    {
        return -1;
    }

    if (auto match = std::ranges::find_if(enm, [](const auto& base) { return base.index() == 2; }); match != enm.end())
    {
        if (match->index() != 2) // Incorrect match.
            return -2;
    }
    else
    {
        // No match.
        return -3;
    }

    for (int i = 2; const auto & base : enm)
        if (base.index() != i)
            return -4;
        else
            i += 2;

    auto bars2 = bars;

    enm = std::move(bars2) |
        std::views::transform([](const auto& bar) -> const Base& { return bar; }) |
        std::views::filter([](const auto& base) { return base.index() % 2 == 1; });

    try
    {
        std::ranges::for_each(enm, [i = 1](const auto& base) mutable {
            if (i != base.index())
                throw;
            else
                i += 2;
        });
    }
    catch (...)
    {
        return -5;
    }

    return 0;
}