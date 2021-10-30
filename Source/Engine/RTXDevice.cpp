#include "RTXDevice.h"
#include <cstdio>

void ErrorFunction(void* userPtr, RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
}

RTXDevice::RTXDevice()
	: EmbreeDevice(nullptr)
{
	/*
	*	Initialization. All of this may fail, but we will be notified by
	*	our errorFunction.
	*/
	EmbreeDevice = rtcNewDevice(nullptr);

	if (!EmbreeDevice)
	{
		printf("error %d: cannot create device\n", rtcGetDeviceError(nullptr));
	}

	rtcSetDeviceErrorFunction(EmbreeDevice, ErrorFunction, nullptr);
}

RTXDevice::~RTXDevice()
{
	if (EmbreeDevice)
	{
		rtcReleaseDevice(EmbreeDevice);
	}
}