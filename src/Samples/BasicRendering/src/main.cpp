#include "SampleApp.h"

#include <iostream>

int main(int argc, char** argv) 
{
	try
	{
		App& app = SampleApp();
		app.start(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}