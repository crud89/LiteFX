#include "SampleApp.h"

#include <iostream>

int main(int argc, char** argv) 
{
	try
	{
		CLiteFxApp& app = SampleApp();
		app.start(argc, argv);
	}
	catch (std::exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
	}
}