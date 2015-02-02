//
// Tomato ASIK
// Ƶ�׷���
//
// (c) 2014 SunnyCase
// ��������: 2015-01-28
#pragma once
#include "../platform.h"

NSDEF_ASIK_CORE

// Ƶ�׷���
class spectrogram
{
public:
	virtual void ASIKCALL set_input(const short* input, size_t input_len) = 0;
	virtual size_t ASIKCALL get_fft_size() const noexcept = 0;
	virtual void ASIKCALL set_fft_size(size_t value) = 0;
	virtual size_t ASIKCALL get_step_size() const noexcept = 0;
	virtual void ASIKCALL set_step_size(size_t value) = 0;
	virtual void ASIKCALL draw() = 0;
	virtual std::vector<uint32_t> ASIKCALL get_output(size_t& width, size_t& height) = 0;
};

NSED_ASIK_CORE

extern "C" ASIKAPI void ASIKCALL CreateSpectrogram(std::unique_ptr<NS_ASIK_CORE::spectrogram>& spec);