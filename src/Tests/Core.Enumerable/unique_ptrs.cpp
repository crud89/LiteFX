#include "common.h"

int main(int argc, char* argv[])
{
	Array<UniquePtr<Foo>> foos;
	Array<UniquePtr<Bar>> bars;
	foos.push_back(std::make_unique<Foo>(0));
	foos.push_back(std::make_unique<Foo>(1));
	foos.push_back(std::make_unique<Foo>(2));
	bars.push_back(std::make_unique<Bar>(3));
	bars.push_back(std::make_unique<Bar>(4));
	bars.push_back(std::make_unique<Bar>(5));

	// NOTE: views::merge unfortunately is only considered for C++26, so we cannot have a single Enumerable<UniquePtr<Base>> for both source containers.
	Enumerable<UniquePtr<Base>> fooBases = foos | std::views::as_rvalue;
	Enumerable<UniquePtr<Base>> barBases = bars | std::views::as_rvalue;

	int i = 0;
	for (auto& base : fooBases)
		if (base->index() != i++)
			return -1;

	for (auto& base : barBases)
		if (base->index() != i++)
			return -2;

	for (auto& foo : foos)
		if (foo != nullptr)
			return -3;

	for (auto& bar : bars)
		if (bar != nullptr)
			return -4;

	Enumerable<UniquePtr<Base>> moreBases = fooBases | std::views::drop(1) | std::views::as_rvalue;
	
	i = 1;
	for (auto& base : moreBases)
		if (base->index() != i++)
			return -5;
}