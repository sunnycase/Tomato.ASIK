//
// Tomato ASIK
// 样本实现
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-06
#pragma once
#include "../include/core/sample.h"

NSDEF_ASIK_CORE

class sample_impl : public sample
{
public:
	sample_impl(const concurrency::array_view<uint32_t, 2>& spectroSection);

	virtual std::vector<uint32_t> ASIKCALL get_data() const;

	__declspec(property(get = get_view)) const concurrency::array_view<uint32_t, 2>& view;
	const concurrency::array_view<uint32_t, 2>& get_view() const noexcept;

	virtual size_t ASIKCALL get_width() const;
	virtual size_t ASIKCALL get_height() const;
private:
	concurrency::array_view<uint32_t, 2> spectroSection;
};

NSED_ASIK_CORE