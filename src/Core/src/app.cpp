#include <litefx/core_types.hpp>

using namespace LiteFX;

App::App() noexcept
{
}

int App::start(const int argc, const char** argv)
{
	return this->start(Array<String>(argv, argv + argc));
}