//
// Tomato ASIK
// CK Distance 服务
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-08
#pragma once
#include "sample.h"

NSDEF_ASIK_CORE

class ck_distance_service
{
public:
	virtual ~ck_distance_service(){}

	virtual float ASIKCALL compute(sample* sampleA, sample* sampleB) = 0;
};

NSED_ASIK_CORE

extern "C" ASIKAPI void ASIKCALL CreateCKDistanceService(size_t height, std::unique_ptr<NS_ASIK_CORE::ck_distance_service>& service);