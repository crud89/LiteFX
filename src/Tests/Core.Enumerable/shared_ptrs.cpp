#include "common.h"

int main(int argc, char* argv[])
{
	Array<SharedPtr<Foo>> foos;
	Array<SharedPtr<Bar>> bars;
	foos.push_back(std::make_shared<Foo>(0));
	foos.push_back(std::make_shared<Foo>(1));
	foos.push_back(std::make_shared<Foo>(2));
	bars.push_back(std::make_shared<Bar>(3));
	bars.push_back(std::make_shared<Bar>(4));
	bars.push_back(std::make_shared<Bar>(5));

	// NOTE: views::merge unfortunately is only considered for C++26, so we cannot have a single Enumerable<UniquePtr<Base>> for both source containers.
	Enumerable<SharedPtr<Base>> fooBases = foos | std::views::drop(1);
	Enumerable<SharedPtr<Base>> barBases = bars | std::views::reverse | std::views::drop(1);

	int i = 1;
	for (auto& base : fooBases)
		if (base->index() != i++)
			return -1;

	i = 4;
	for (auto& base : barBases)
		if (base->index() != i--)
			return -2;

	Enumerable<SharedPtr<Base>> moreBases = fooBases | std::views::drop(1) | std::views::take(1);

	i = 2;
	for (auto& base : moreBases)
		if (base->index() != i++)
			return -3;

	// Test reference counts.
	i = 1;
	for (auto& foo : foos)
		if (foo.use_count() != i++)
			return -4;

	moreBases = barBases | std::views::drop(1) | std::views::take(1);

	i = 3;
	for (auto& bar : bars)
		if (bar.use_count() != i--)
			return -5;
}