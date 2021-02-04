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
	static const int TILE_SIZE = 32;

	RECT Rect;
	std::vector<Spectrum> Data;
};

class Integrator
{
public:
	// Tile info for multi threading
	static constexpr int NumChannels = 3;

	// Output image resolution
	static constexpr int Width = 1920;
	static constexpr int Height = 1080;

	int Render(Scene& Scene, Sampler& Sampler);

	// All integrator inherited needs to implement this method
	/*
	*	Sample the incident radiance along the given ray
	*/
	virtual Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) = 0;
};