#include "Integrator.h"
#include "../Texture2D.h"
#include "../Scene.h"
#include "../Sampler/Sampler.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <future>

#include <ppl.h>
using namespace concurrency;

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define MULTI_THREADED 1

#define DEBUG_X		   600
#define DEBUG_Y		   100
static bool DEBUG_PIXEL = false;

template<typename T, typename U, typename V>
inline T Clamp(T val, U low, V high)
{
	if (val < low)
		return low;
	if (val > high)
		return high;
	return val;
}

inline float GammaCorrect(float value)
{
	if (value <= 0.0031308f)
	{
		return 12.92f * value;
	}
	return 1.055f * std::pow(value, (1.f / 2.4f)) - 0.055f;
}

int Save(const Texture2D<RGBSpectrum>& Image)
{
	constexpr int NumChannels = 3;

	// Saves a input image as a png using stb
	std::unique_ptr<BYTE[]> Pixels = std::make_unique<BYTE[]>(Image.Width * Image.Height * NumChannels);

	BYTE* pDst = Pixels.get();

	int index = 0;
	for (int y = int(Image.Height) - 1; y >= 0; --y)
	{
		for (int x = 0; x < int(Image.Width); ++x)
		{
			auto color = Image.GetPixel(x, y);

#define TO_BYTE(v) (uint8_t) Clamp(255.f * GammaCorrect(v) + 0.5f, 0.f, 255.f)
			pDst[0] = TO_BYTE(color[0]);
			pDst[1] = TO_BYTE(color[1]);
			pDst[2] = TO_BYTE(color[2]);
#undef TO_BYTE
			pDst += 3;
		}
	}

#ifdef _DEBUG
	const char* Name = "Debug.png";
#else
	const char* Name = "Release.png";
#endif
	if (stbi_write_png(Name, Image.Width, Image.Height, 3, Pixels.get(), Image.Width * NumChannels))
	{
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

class ProgressReport
{
public:
	ProgressReport(const std::string& Header, int TotalProgress)
		: Header(Header)
		, TotalProgress(TotalProgress)
	{
		Thread = std::thread(
			[this]()
			{
				Print();
			});
	}

	~ProgressReport()
	{
		Shutdown = true;
		Thread.join();
		printf("\n");
	}

	static int TerminalWidth()
	{
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (h == INVALID_HANDLE_VALUE || !h)
		{
			fprintf(stderr, "GetStdHandle() call failed");
			return 80;
		}
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
		GetConsoleScreenBufferInfo(h, &bufferInfo);
		return bufferInfo.dwSize.X;
	}

	void Update(int NewProgress = 1) { CurrentProgress += NewProgress; }

	void Print()
	{
		auto barLength		 = TerminalWidth() - 28;
		auto totalPlusses	 = std::max(2, barLength - (int)Header.size());
		auto progressPrinted = 0;

		// Initialize progress string
		auto					bufLen = Header.size() + totalPlusses + 64;
		std::unique_ptr<char[]> buf(new char[bufLen]);
		snprintf(buf.get(), bufLen, "\r%s: [", Header.c_str());
		char* curSpace = buf.get() + strlen(buf.get());
		char* s		   = curSpace;
		for (int i = 0; i < totalPlusses; ++i)
			*s++ = ' ';
		*s++ = ']';
		*s++ = ' ';
		*s++ = '\0';
		fputs(buf.get(), stdout);
		fflush(stdout);

		std::chrono::milliseconds sleepDuration(250);
		int						  iterCount = 0;
		while (!Shutdown)
		{
			std::this_thread::sleep_for(sleepDuration);

			// Periodically increase sleepDuration to reduce overhead of
			// updates.
			++iterCount;
			if (iterCount == 10)
				// Up to 0.5s after ~2.5s elapsed
				sleepDuration *= 2;
			else if (iterCount == 70)
				// Up to 1s after an additional ~30s have elapsed.
				sleepDuration *= 2;
			else if (iterCount == 520)
				// After 15m, jump up to 5s intervals
				sleepDuration *= 5;

			float percentDone	 = float(CurrentProgress) / float(TotalProgress);
			int	  ProgressNeeded = (int)std::round(totalPlusses * percentDone);
			while (progressPrinted < ProgressNeeded)
			{
				*curSpace++ = '=';
				++progressPrinted;
			}
			fputs(buf.get(), stdout);

			printf(" %i %%", int(std::min(percentDone * 100.0f, 100.0f)));

			fflush(stdout);
		}
	}

private:
	std::string		 Header;
	std::atomic<int> TotalProgress;

	std::atomic<int>  CurrentProgress = 0;
	std::atomic<bool> Shutdown		  = false;
	std::thread		  Thread;
};

void Integrator::Initialize(Scene& Scene)
{
	TileManager.Initialize(Width, Height);

	Scene.Camera.AspectRatio = float(Width) / float(Height);
	Scene.Generate();
}

int Integrator::Render(const Scene& Scene, const Sampler& Sampler)
{
	Texture2D<RGBSpectrum> Output(Width, Height);

	ProgressReport ProgressReport("Render", (int)TileManager.size());

	auto Process = [&](FilmTile& Tile)
	{
		auto Rect = Tile.Rect;

		auto pSampler = Sampler.Clone();

		// Render
		// For each pixel and pixel sample
		for (int y = Rect.top; y < Rect.bottom; ++y)
		{
			for (int x = Rect.left; x < Rect.right; ++x)
			{
				if (x == DEBUG_X && y == DEBUG_Y)
				{
					DEBUG_PIXEL = true;
				}

				pSampler->StartPixel(x, y);

				Spectrum L(0);
				do
				{
					auto sampleJitter = pSampler->Get2D();

					auto u = (float(x) + sampleJitter.x) / (float(Width) - 1);
					auto v = (float(y) + sampleJitter.y) / (float(Height) - 1);

					RayDesc ray = Scene.Camera.GetRay(u, v);

					L += Li(ray, Scene, *pSampler);
				} while (pSampler->StartNextSample());

				L /= float(pSampler->GetNumSamplesPerPixel());

				Output.SetPixel(x, y, L);
			}
		}

		ProgressReport.Update();
	};
#if MULTI_THREADED
	parallel_for_each(
		TileManager.begin(),
		TileManager.end(),
		[&](auto& tile)
		{
			Process(tile);
		});
#else
	for (auto& tile : TileManager)
	{
		Process(tile);
	}
#endif

	return Save(Output);
}

Spectrum EstimateDirect(
	const Interaction& Interaction,
	const Light&	   Light,
	const Vector2f&	   XiLight,
	const Scene&	   Scene,
	Sampler&		   Sampler,
	bool			   HandleMedia)
{
	Spectrum Ld(0.0f);
	// Sample light source with multiple importance sampling
	Vector3f		 wi;
	float			 lightPdf = 0.0f, scatteringPdf = 0.0f;
	VisibilityTester visibility;
	Spectrum		 Li = Light.SampleLi(Interaction, XiLight, &wi, &lightPdf, &visibility);
	if (lightPdf > 0.0f && !Li.IsBlack())
	{
		// Compute BSDF or phase function's value for light sample
		Spectrum f;
		if (Interaction.IsSurfaceInteraction())
		{
			// Evaluate BSDF for light sampling strategy
			const SurfaceInteraction& si = static_cast<const SurfaceInteraction&>(Interaction);
			f							 = si.BSDF.f(si.wo, wi) * absdot(wi, si.ShadingFrame.n);
			scatteringPdf				 = si.BSDF.Pdf(si.wo, wi);
		}
		else
		{
			// Evaluate phase function for light sampling strategy
			const MediumInteraction& mi = static_cast<const MediumInteraction&>(Interaction);
			float					 p	= mi.phase->p(mi.wo, wi);
			f							= Spectrum(p);
			scatteringPdf				= p;
		}

		// Evaluate BSDF for light sampling strategy

		if (!f.IsBlack())
		{
			// Compute effect of visibility for light source sample
			if (HandleMedia)
			{
				Li *= visibility.Tr(Scene, Sampler);
			}
			else
			{
				if (!visibility.Unoccluded(Scene))
				{
					Li = Spectrum(0.0f);
				}
			}

			// Add light's contribution to reflected radiance
			if (!Li.IsBlack())
			{
				// Only handling point light for now
				Ld += f * Li / lightPdf;
			}
		}
	}

	return Ld;
}

Spectrum Integrator::UniformSampleOneLight(
	const Interaction& Interaction,
	const Scene&	   Scene,
	Sampler&		   Sampler,
	bool			   HandleMedia)
{
	if (Scene.Lights.empty())
	{
		return Spectrum(0.0f);
	}

	int numLights = (int)Scene.Lights.size();

	int	  lightIndex;
	float lightPdf;

	lightIndex = std::min((int)(Sampler.Get1D() * numLights), numLights - 1);
	lightPdf   = 1.0f / float(numLights);

	const auto pLight = Scene.Lights[lightIndex];

	Vector2f Xi = Sampler.Get2D();

	return EstimateDirect(Interaction, *pLight, Xi, Scene, Sampler, HandleMedia) / lightPdf;
}
