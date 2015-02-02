//
// Tomato ASIK
// 频谱分析
//
// (c) 2014 SunnyCase
// 创建日期: 2015-01-28
#pragma once
#include "../include/core/spectrogram.h"
#include <amp_graphics.h>

NSDEF_ASIK_CORE

// 频谱分析实现
class spectrogram_impl : public spectrogram
{
public:
	spectrogram_impl();

	virtual void ASIKCALL set_input(const short* input, size_t input_len);
	virtual size_t ASIKCALL get_fft_size() const noexcept;
	virtual void ASIKCALL set_fft_size(size_t value);
	virtual size_t ASIKCALL get_step_size() const noexcept;
	virtual void ASIKCALL set_step_size(size_t value);
	virtual void ASIKCALL draw();
	virtual std::vector<uint32_t> ASIKCALL get_output(size_t& width, size_t& height);
private:
	void sample_fft_input(size_t step);
private:
	size_t fft_size;
	size_t step_size;
	size_t width;
	size_t height;
	std::unique_ptr<concurrency::array<float, 1>> input_buffer;
	std::unique_ptr<concurrency::graphics::texture<concurrency::graphics::uint_4, 2>> output_buffer;
	concurrency::extent<1> fft_buffer_ext;
	concurrency::array<float, 1> fft_input_buffer;
	concurrency::array<std::complex<float>, 1> fft_output_buffer;
	std::unique_ptr<fft<float, 1>> fft_transform;
};

NSED_ASIK_CORE
