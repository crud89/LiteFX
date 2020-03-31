#include <litefx/core_types.hpp>

using namespace LiteFX;

App::App()
{
}

App::~App()
{
	this->stop();
}

int App::start(int argc, char** argv)
{
	return this->start(Array<String>(argv, argv + argc));
}

int App::start(const Array<String>& args)
{
	this->work();

	return EXIT_SUCCESS;
}

void App::stop()
{
}