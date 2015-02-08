//
// Tomato ASIK
// 样本实现
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-06
#include "stdafx.h"
#include "sample_impl.h"

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;

sample_impl::sample_impl(const array_view<uint32_t, 2>& spectroSection)
	:spectroSection(spectroSection)
{
	
}

std::vector<uint32_t> ASIKCALL sample_impl::get_data() const
{
	auto size = spectroSection.extent.size();

	std::vector<uint32_t> data(size);
	copy(spectroSection, stdext::make_unchecked_array_iterator(data.data()));
	return std::move(data);
}

const array_view<uint32_t, 2>& sample_impl::get_view() const noexcept
{
	return spectroSection;
}

size_t ASIKCALL sample_impl::get_width() const
{
	return spectroSection.extent[1];
}

size_t ASIKCALL sample_impl::get_height() const
{
	return spectroSection.extent[0];
}
