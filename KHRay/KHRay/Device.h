#pragma once
#include <embree/rtcore.h>

class Device
{
public:
	Device();
	~Device();

	operator auto() const
	{
		return m_Device;
	}
private:
	RTCDevice m_Device;
};