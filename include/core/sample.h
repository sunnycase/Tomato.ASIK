//
// Tomato ASIK
// 样本
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-06
#pragma once
#include "../platform.h"

NSDEF_ASIK_CORE

class sample
{
public:
	sample(const byte* src, size_t freq_extent, size_t time_extent, size_t start_time = 0)
		:_src(src + freq_extent * start_time), _freq_extent(freq_extent), _time_extent(time_extent)
	{

	}

	__declspec(property(get = get_data)) const byte* data;
	const byte* get_data() const { return _src; }

	_declspec(property(get = get_freq_extent)) size_t freq_extent;
	size_t get_freq_extent() const { return _freq_extent; }

	_declspec(property(get = get_time_extent)) size_t time_extent;
	size_t get_time_extent() const { return _time_extent; }

	_declspec(property(get = get_data_length)) size_t data_length;
	size_t get_data_length() const { return freq_extent * time_extent; }
private:
	const byte* _src;
	const size_t _freq_extent, _time_extent;
};

NSED_ASIK_CORE