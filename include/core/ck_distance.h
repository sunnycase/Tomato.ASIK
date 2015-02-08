//
// Tomato ASIK
// CK Distance
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-03
#pragma once
#include "../platform.h"
#include "spectrogram.h"

NSDEF_ASIK_CORE

class ck_distance
{
public:
	virtual float ASIKCALL compute(sample* sampleA, sample* sampleB) = 0;
};

NSED_ASIK_CORE

extern "C" ASIKAPI void ASIKCALL CreateCKDistance(std::unique_ptr<NS_ASIK_CORE::ck_distance>& dist, size_t width, size_t height);