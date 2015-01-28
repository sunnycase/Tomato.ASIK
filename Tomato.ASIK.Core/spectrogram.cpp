//
// Tomato ASIK
// 频谱分析
//
// (c) 2014 SunnyCase
// 创建日期: 2015-01-28
#include "stdafx.h"
#include "spectrogram_impl.h"
#include <amp_math.h>
#include <comdef.h>

#ifdef NDEBUG
#ifdef _X86_
#pragma comment(lib, "../vendors/ampfft/x86/lib/amp_fft.lib")
#elif _AMD64_
#pragma comment(lib, "../vendors/ampfft/x64/lib/amp_fft.lib")
#endif
#else
#ifdef _X86_
#pragma comment(lib, "../vendors/ampfft/x86/lib/amp_fftd.lib")
#elif _AMD64_
#pragma comment(lib, "../vendors/ampfft/x64/lib/amp_fftd.lib")
#endif
#endif

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;
namespace ampmath = concurrency::fast_math;

#define EPSILON 1.0e-6f

spectrogram_impl::spectrogram_impl()
	:fft_size(1024), step_size(512), fft_buffer_ext(fft_size),
	fft_input_buffer(fft_buffer_ext), fft_output_buffer(fft_buffer_ext),
	fft_transform(std::make_unique<fft<float, 1>>(fft_buffer_ext))
{

}

void ASIKCALL spectrogram_impl::set_input(const short* input, size_t input_len)
{
	auto data = std::make_unique<float[]>(input_len);
	auto end = data.get() + input_len;
	for (auto p = data.get(); p != end; ++p)
		*p = *input++ / 32767.9f;
	input_buffer = std::make_unique<array<float, 1>>(extent<1>(input_len), data.get());
}

size_t ASIKCALL spectrogram_impl::get_fft_size() const noexcept
{
	return fft_size;
}

void ASIKCALL spectrogram_impl::set_fft_size(size_t value)
{
	fft_size = value;
	fft_buffer_ext[0] = value;
	fft_input_buffer = array<float, 1>(fft_buffer_ext);
	fft_output_buffer = array<std::complex<float>, 1>(fft_buffer_ext);
	fft_transform = std::make_unique<fft<float, 1>>(fft_buffer_ext);
}

size_t ASIKCALL spectrogram_impl::get_step_size() const noexcept
{
	return step_size;
}

void ASIKCALL spectrogram_impl::set_step_size(size_t value)
{
	step_size = value;
}

std::vector<float> ASIKCALL spectrogram_impl::draw(size_t& width, size_t& height)
{
	if (!input_buffer)
		throw std::exception("input must be set.");

	width = (input_buffer->extent[0] - fft_size) / step_size;
	height = fft_size / 2;
	array<float, 2> output(extent<2>(width, height));
	float scale = 2.0f / fft_size;

	for (size_t x = 0; x < width; x++)
	{
		sample_fft_input(x);
		fft_transform->forward_transform(fft_input_buffer, fft_output_buffer);
		array_view<std::complex<float>, 1> fft_output_view(fft_output_buffer);
		auto output_view = output[x];
		parallel_for_each(extent<1>(std::min(height, step_size)),
			[fft_output_view, output_view, scale](index<1> idx) restrict(amp)
		{
			auto real = fft_output_view[idx]._Val[_RE] * scale;
			auto img = fft_output_view[idx]._Val[_IM] * scale;
			auto dB = -10.0f * ampmath::log10((real * real + img * img) + EPSILON);
			// 限制在 0 - 60dB
			output_view[idx] = 1.0f - ampmath::fmin(1.0f, ampmath::fmax(dB / 60.0f, 0.0f));
		});
	}

	return output;
}

void spectrogram_impl::sample_fft_input(size_t step)
{
	auto begin = step * step_size;
	input_buffer->section(begin, fft_size).copy_to(fft_input_buffer);
}

void ASIKCALL CreateSpectrogram(std::unique_ptr<NS_ASIK_CORE::spectrogram>& spec)
{
	spec = std::make_unique<spectrogram_impl>();
}