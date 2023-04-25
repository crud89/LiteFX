#include "common.h"

int main(int argc, char* argv[])
{
	Array<Test> tests{ Test{ 1, "First" }, Test{ 2, "Second" }, Test{ 3, "Third" } };

	int i = 1;
	for (auto& test : tests)
		if (test.index != i++)
			return -1;

	Enumerable<Ref<Test>> testRefs = tests | std::views::transform([](auto& test) -> Ref<Test> { return test; });
	i = 1;
	for (auto& test : testRefs)
		if (test.get().index != i++)
			return -2;

	Enumerable<Ref<Test>> reversedRefs = tests | std::views::transform([](auto& test) -> Ref<Test> { return test; }) | std::views::reverse;
	i = 3;
	for (auto& test : reversedRefs)
		if (test.get().index != i--)
			return -3;

	tests.front().index = 4;
	tests.back().index = 6;

	if (testRefs.front().get().index != 4)
		return -4;

	if (reversedRefs.front().get().index != 6)
		return -5;
}