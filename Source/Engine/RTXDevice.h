#pragma once
#include <embree/rtcore.h>

class RTXDevice
{
public:
	RTXDevice();
	~RTXDevice();

	[[nodiscard]] operator RTCDevice() const noexcept { return EmbreeDevice; }

private:
	RTCDevice EmbreeDevice;
};
