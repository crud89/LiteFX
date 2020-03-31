#include "sample.h"

#include <iostream>

int main(int argc, char** argv) 
{
	try
	{
		App& app = SampleApp();
		app.start(argc, argv);
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}