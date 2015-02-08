//
// Tomato ASIK
// 频谱分析
//
// (c) 2014 SunnyCase
// 创建日期: 2015-01-28
#pragma once
#include "../platform.h"
#include "sample.h"

NSDEF_ASIK_CORE

// 频谱分析
class spectrogram
{
public:
	virtual void ASIKCALL set_input(const short* input, size_t input_len) = 0;
	virtual size_t ASIKCALL get_fft_size() const noexcept = 0;
	virtual void ASIKCALL set_fft_size(size_t value) = 0;
	virtual size_t ASIKCALL get_step_size() const noexcept = 0;
	virtual void ASIKCALL set_step_size(size_t value) = 0;
	virtual void ASIKCALL draw() = 0;
	virtual size_t ASIKCALL get_length() = 0;
	virtual std::unique_ptr<sample> ASIKCALL get_sample(size_t startIndex, size_t length) = 0;

	std::unique_ptr<sample> get_sample()
	{
		return get_sample(0, get_length());
	}
};

NSED_ASIK_CORE

extern "C" ASIKAPI void ASIKCALL CreateSpectrogram(std::unique_ptr<NS_ASIK_CORE::spectrogram>& spec);