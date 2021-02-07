#pragma once
static constexpr float g_PI = 3.141592654f;
static constexpr float g_2PI = 6.283185307f;
static constexpr float g_1DIVPI = 0.318309886f;
static constexpr float g_1DIV2PI = 0.159154943f;
static constexpr float g_PIDIV2 = 1.570796327f;
static constexpr float g_PIDIV4 = 0.785398163f;

#include "Vector2.h"
#include "Vector3.h"
#include "Transform.h"

// https://github.com/mmp/pbrt-v3/blob/master/src/core/pbrt.h
inline uint32_t FloatToBits(float f)
{
	uint32_t ui;
	memcpy(&ui, &f, sizeof(float));
	return ui;
}

inline float BitsToFloat(uint32_t ui)
{
	float f;
	memcpy(&f, &ui, sizeof(uint32_t));
	return f;
}

inline float NextFloatUp(float v)
{
	// Handle infinity and negative zero for _NextFloatUp()_
	if (std::isinf(v) && v > 0.) return v;
	if (v == -0.f) v = 0.f;

	// Advance _v_ to next higher float
	uint32_t ui = FloatToBits(v);
	if (v >= 0)
		++ui;
	else
		--ui;
	return BitsToFloat(ui);
}

inline float NextFloatDown(float v)
{
	// Handle infinity and positive zero for _NextFloatDown()_
	if (std::isinf(v) && v < 0.) return v;
	if (v == 0.f) v = -0.f;
	uint32_t ui = FloatToBits(v);
	if (v > 0)
		--ui;
	else
		++ui;
	return BitsToFloat(ui);
}