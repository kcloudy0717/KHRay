#include "Device.h"
#include <cstdio>

void ErrorFunction(void* userPtr, RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
}

Device::Device()
	: m_Device(nullptr)
{
	/*
	*	Initialization. All of this may fail, but we will be notified by
	*	our errorFunction.
	*/
	m_Device = rtcNewDevice(nullptr);

	if (!m_Device)
	{
		printf("error %d: cannot create device\n", rtcGetDeviceError(nullptr));
	}

	rtcSetDeviceErrorFunction(m_Device, ErrorFunction, nullptr);
}

Device::~Device()
{
	if (m_Device)
	{
		rtcReleaseDevice(m_Device);
	}
}