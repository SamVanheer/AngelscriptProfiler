#include <SDL2/SDL_main.h>

#include "Application.hpp"

int main(int argc, char** argv)
{
	Application app;

	return app.Run({ const_cast<const char**>(argv), static_cast<std::size_t>(argc) });
}
