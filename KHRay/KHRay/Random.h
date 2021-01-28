#pragma once
#include <type_traits>
#include <random>

inline float random()
{
	static std::uniform_real_distribution<float> distribution(0, 1);
	static std::mt19937 generator;
	return distribution(generator);
}