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
	virtual ~sample() {}

	__declspec(property(get = get_data)) std::vector<uint32_t> data;
	virtual std::vector<uint32_t> ASIKCALL get_data() const = 0;

	_declspec(property(get = get_width)) size_t width;
	virtual size_t ASIKCALL get_width() const = 0;

	_declspec(property(get = get_height)) size_t height;
	virtual size_t ASIKCALL get_height() const = 0;
};

NSED_ASIK_CORE