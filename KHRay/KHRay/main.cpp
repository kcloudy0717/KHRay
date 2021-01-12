// main.cpp : Defines the entry point for the application.
//
#if defined(_DEBUG)
// memory leak
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#define ENABLE_LEAK_DETECTION() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#define SET_LEAK_BREAKPOINT(x) _CrtSetBreakAlloc(x)
#else
#define ENABLE_LEAK_DETECTION() 0
#define SET_LEAK_BREAKPOINT(X) X
#endif

#include <glm/glm.hpp>

#include <gli/texture2d.hpp>
#include <gli/save.hpp>

const int Width = 1280;
const int Height = 720;

int main(int argc, char** argv)
{
#if defined(_DEBUG)
	ENABLE_LEAK_DETECTION();
	SET_LEAK_BREAKPOINT(-1);
#endif

	gli::texture2d output = gli::texture2d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(Width, Height), 1);

	return EXIT_SUCCESS;
}