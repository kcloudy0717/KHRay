#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <memory>
#include <vector>

#include "../Spectrum.h"

struct Ray;
struct Scene;
class Sampler;

struct FilmTile
{
	RECT Rect;
	std::vector<Spectrum> Data;
};

class Integrator
{
public:
	// Tile info for multi threading
	static constexpr int NumXTiles = 3;
	static constexpr int NumYTiles = 3;
	static constexpr int NumTiles = NumXTiles * NumYTiles;
	static constexpr int NumChannels = 3;

	// Output image resolution
	static constexpr int Width = 1920;
	static constexpr int Height = 1080;

	int Render(Scene& Scene, Sampler& Sampler);

	// All integrator inherited needs to implement this method
	/*
	*	Sample the incident radiance along the given ray
	*/
	virtual Spectrum Li(const Ray& Ray, const Scene& Scene, Sampler& Sampler) = 0;
};