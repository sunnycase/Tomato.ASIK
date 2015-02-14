//
// Tomato ASIK
// CK Distance 服务
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-08
#include "stdafx.h"
#include "ck_distance_service_impl.h"

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;

ck_distance_service_impl::ck_distance_service_impl(size_t freq_extent)
	:freq_extent(freq_extent)
{
	// 16 的倍数，且大于 128
	if (freq_extent < 128 || freq_extent % 16 != 0)
		throw std::exception("Frequceny extent is not compatible with H.264.");
}

ck_distance * ck_distance_service_impl::acquire_ck_distance_instance(size_t time_extent)
{
	auto it = ck_instances.find(time_extent);
	if (it != ck_instances.end())
		return it->second.get();

	return ck_instances.emplace(time_extent,
		std::make_unique<ck_distance>(freq_extent, time_extent)).first->second.get();
}

float ASIKCALL ck_distance_service_impl::compute(const sample& sampleA, const sample& sampleB)
{
	if (sampleA.freq_extent != freq_extent || sampleB.freq_extent != freq_extent)
		throw std::exception("freq extent is not match.");
	if (sampleA.time_extent != sampleB.time_extent)
		throw std::exception("time extent is not match.");

	auto ck_inst = acquire_ck_distance_instance(sampleA.time_extent);
	return ck_inst->compute(sampleA, sampleB);
}

void ASIKCALL CreateCKDistanceService(size_t height, std::unique_ptr<ck_distance_service>& service)
{
	service = std::make_unique<ck_distance_service_impl>(height);
}