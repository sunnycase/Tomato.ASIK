//
// Tomato ASIK
// CK Distance 服务实现
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-08
#pragma once
#include "../include/core/ck_distance_service.h"
#include "ck_distance.h"

namespace std
{
	template<>
	struct hash<std::pair<NS_ASIK_CORE::sample*, size_t>>
	{
		size_t operator()(const std::pair<NS_ASIK_CORE::sample*, size_t>& value) const
		{
			return hash<NS_ASIK_CORE::sample*>()(value.first) |
				hash<size_t>()(value.second);
		}
	};
}

NSDEF_ASIK_CORE

class ck_distance_service_impl : public ck_distance_service
{
	typedef std::pair<sample*, size_t> sample_cache_item_t;
public:
	ck_distance_service_impl(size_t freq_extent);

	virtual float ASIKCALL compute(const sample& sampleA, const sample& sampleB);
private:
	ck_distance* acquire_ck_distance_instance(size_t time_extent);
private:
	const size_t freq_extent;
	std::unordered_map<size_t, std::unique_ptr<ck_distance>> ck_instances;
	//std::unordered_map<sample_cache_item_t, std::unique_ptr<sample_impl>> sample_cache;
};

NSED_ASIK_CORE