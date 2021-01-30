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
	static constexpr INT NumXTiles = 3;
	static constexpr INT NumYTiles = 3;
	static constexpr INT NumTiles = NumXTiles * NumYTiles;
	static constexpr INT NumChannels = 3;

	// Output image resolution
	static constexpr INT Width = 1920;
	static constexpr INT Height = 1080;

	int Render(Scene& Scene, Sampler& Sampler);

	// All integrator inherited needs to implement this method
	virtual Spectrum Li(const Ray& Ray, const Scene& Scene) = 0;
};

class PathIntegrator : public Integrator
{
public:
	PathIntegrator(int MaxDepth);

	Spectrum Li(const Ray& Ray, const Scene& Scene) override;
private:
	int MaxDepth;
};

std::unique_ptr<PathIntegrator> CreatePathIntegrator(int MaxDepth);