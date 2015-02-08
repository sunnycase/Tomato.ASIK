//
// Tomato ASIK
// 频谱分析
//
// (c) 2014 SunnyCase
// 创建日期: 2015-01-28
#pragma once
#include "../include/core/spectrogram.h"

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
	virtual size_t ASIKCALL get_length();
	virtual std::unique_ptr<sample> ASIKCALL get_sample(size_t startIndex, size_t length);

	concurrency::array_view<uint32_t, 2> get_section() const;
private:
	void sample_fft_input(size_t step);
	void produce_spec_texture(concurrency::array_view<float, 2> tmp_output);
	void ensure_spectorgram_created();
private:
	size_t fft_size;
	size_t step_size;
	size_t width;
	size_t height;
	std::unique_ptr<concurrency::array<float, 1>> input_buffer;
	std::unique_ptr<concurrency::array<uint32_t, 2>> output_buffer;
	concurrency::extent<1> fft_buffer_ext;
	concurrency::array<float, 1> fft_input_buffer;
	concurrency::array<std::complex<float>, 1> fft_output_buffer;
	std::unique_ptr<fft<float, 1>> fft_transform;
};

NSED_ASIK_CORE
