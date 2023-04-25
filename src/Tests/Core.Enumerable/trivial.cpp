#include "common.h"

int main(int argc, char* argv[])
{
	Array<int> integers{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	Enumerable<int> enumInts = integers;
	
	int i;
	for (i = 0; i < integers.size(); i++)
		if (integers[i] != i)
			return -1;

	i = 0;
	for (auto it = enumInts.begin(); it != enumInts.end(); ++it, ++i)
		if (*it != i)
			return -2;

	Enumerable<int> otherInts = enumInts;

	i = 0;
	for (auto it = enumInts.begin(); it != enumInts.end(); ++it, ++i)
		if (*it != i)
			return -3;

	i = 0;
	for (auto it = otherInts.begin(); it != otherInts.end(); ++it, ++i)
		if (*it != i)
			return -4;

	Array<Test> tests{ Test{ 1, "First" }, Test{ 2, "Second" }, Test{ 3, "Third" } };
	Enumerable<Test> enumTests = tests;
	Enumerable<Test> otherTests = enumTests;

	i = 1;
	for (auto& test : tests)
		if (test.index != i++)
			return -5;

	i = 1;
	for (auto& test : enumTests)
		if (test.index != i++)
			return -6;

	i = 1;
	for (auto& test : otherTests)
		if (test.index != i++)
			return -7;

	Enumerable<int> reversedInts = integers | std::views::reverse;

	i = 9;
	for (auto it = reversedInts.begin(); it != reversedInts.end(); ++it, --i)
		if (*it != i)
			return -8;

	Enumerable<int> filteredInts = reversedInts | std::views::drop(3) | std::views::take(5);

	i = 6;
	for (auto it = filteredInts.begin(); it != filteredInts.end(); ++it, --i)
		if (*it != i)
			return -9;

	Array<int> integersVector = filteredInts | std::views::filter([](auto& i) { return i % 2; }) | std::ranges::to<std::vector>();

	i = 5;
	for (auto it = integersVector.begin(); it != integersVector.end(); ++it, i -= 2)
		if (*it != i)
			return -10;
}