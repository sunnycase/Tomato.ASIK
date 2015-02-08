//
// Tomato ASIK
// CK Distance 服务实现
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-08
#pragma once
#include "../include/core/ck_distance_service.h"
#include "ck_distance.h"
#include "sample_impl.h"

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
	ck_distance_service_impl(size_t height);

	virtual float ASIKCALL compute(sample* sampleA, sample* sampleB);
private:
	static size_t make_width_compatible(size_t width) noexcept;

	std::unique_ptr<sample_impl> make_sample_compatible(sample_impl* sample);
	ck_distance* acquire_ck_distance_instance(size_t width);
private:
	const size_t height;
	std::unordered_map<size_t, std::unique_ptr<ck_distance>> ck_instances;
	//std::unordered_map<sample_cache_item_t, std::unique_ptr<sample_impl>> sample_cache;
};

NSED_ASIK_CORE