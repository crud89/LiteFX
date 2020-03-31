#include "litefx/core_types.hpp"

using namespace LiteFX;

App::App()
{
}

App::~App()
{
	this->stop();
}

void App::start(int argc, char** argv)
{
	this->start(Array<String>(argv, argv + argc));
}

void App::start(const Array<String>& args)
{
	this->work();
}

void App::stop()
{
}