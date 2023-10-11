#include "common.h"

int main(int argc, char* argv[])
{
	Array<UniquePtr<Foo>> foos;
	foos.push_back(makeUnique<Foo>(0));
	foos.push_back(makeUnique<Foo>(1));
	foos.push_back(makeUnique<Foo>(2));
	auto bars = []() -> Generator<UniquePtr<Bar>> {
		for (int i(3); ; ++i)
			co_yield makeUnique<Bar>(i);
	}() | std::views::take(3) | std::views::as_rvalue | std::ranges::to<Array<UniquePtr<Base>>>();
	
	// NOTE: views::merge unfortunately is only considered for C++26, so we cannot have a single Enumerable<UniquePtr<Base>> for both source containers.
	Enumerable<UniquePtr<Base>> fooBases = foos | std::views::as_rvalue | std::views::drop(1);
	Enumerable<UniquePtr<Base>> barBases = bars | std::views::reverse | std::views::as_rvalue | std::views::drop(1);

	if (!foos[0] || foos[1] || foos[2])
		return -1;

	if (bars[0] || bars[1] || !bars[2])
		return -2;

	int i = 1;
	for (auto& base : fooBases)
		if (base->index() != i++)
			return -3;

	i = 4;
	for (auto& base : barBases)
		if (base->index() != i--)
			return -4;

	Enumerable<UniquePtr<Base>> moreBases = fooBases | std::views::drop(1) | std::views::as_rvalue;
	
	i = 2;
	for (auto& base : moreBases)
		if (base->index() != i++)
			return -5;
}