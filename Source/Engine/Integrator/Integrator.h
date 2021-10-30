#pragma once
#include <memory>
#include <vector>
#include "../Spectrum.h"

struct Ray;
struct SurfaceInteraction;
struct Scene;
class Sampler;

struct FilmTile
{
	static const int TILE_SIZE = 32;

	RECT Rect;
};

class TileManager
{
public:
	void Initialize(int Width, int Height)
	{
		for (int y = 0; y < Height; y += FilmTile::TILE_SIZE)
		{
			for (int x = 0; x < Width; x += FilmTile::TILE_SIZE)
			{
				int minX = x, minY = y;
				int maxX = x + FilmTile::TILE_SIZE, maxY = y + FilmTile::TILE_SIZE;
				maxX = maxX <= Width ? maxX : Width;
				maxY = maxY <= Height ? maxY : Height;

				FilmTile tile = {};
				tile.Rect	  = { minX, minY, maxX, maxY };

				FilmTiles.emplace_back(std::move(tile));
			}
		}
	}

	auto& operator[](int i) { return FilmTiles[i]; }

	auto size() const { return FilmTiles.size(); }
	auto begin() { return FilmTiles.begin(); }
	auto end() { return FilmTiles.end(); }

private:
	std::vector<FilmTile> FilmTiles;
};

class Integrator
{
public:
	virtual ~Integrator() = default;

	// Output image resolution
	static constexpr int Width	= 1920;
	static constexpr int Height = 1080;

	void Initialize(Scene& Scene);
	int	 Render(const Scene& Scene, const Sampler& Sampler);

	// All integrator inherited needs to implement this method
	/*
	 *	Sample the incident radiance along the given ray
	 */
	virtual Spectrum Li(Ray ray, const Scene& scene, Sampler& sampler) = 0;

	static Spectrum UniformSampleOneLight(const SurfaceInteraction& Interaction, const Scene& scene, Sampler& sampler);

private:
	TileManager TileManager;
};
