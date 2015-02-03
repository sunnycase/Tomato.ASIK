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
#include "ck_distance_impl.h"

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
using namespace concurrency::graphics;
namespace ampmath = concurrency::fast_math;

#define EPSILON 1.0e-6f
#define PI 3.1415926f

spectrogram_impl::spectrogram_impl()
	:fft_size(512), step_size(256), fft_buffer_ext(fft_size),
	fft_input_buffer(fft_buffer_ext),
	fft_output_buffer(fft_buffer_ext),
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

	width = (input_len - fft_size) / step_size;
	height = fft_size / 2;
	auto newWidth = width - width % 16;
	if (newWidth < width) newWidth += 16;
	output_buffer = std::make_unique<graphics::texture<graphics::uint_4, 2>>((int)height, (int)newWidth, 8U);
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

void ASIKCALL spectrogram_impl::draw()
{
	if (!input_buffer)
		throw std::exception("input must be set.");

	float scale = 2.0f / fft_size;
	array<float, 2> tmp_output((int)width, (int)height);

	for (size_t x = 0; x < width; x++)
	{
		sample_fft_input(x);
		fft_transform->forward_transform(fft_input_buffer, fft_output_buffer);
		array_view<std::complex<float>, 1> fft_output_view(fft_output_buffer);
		auto output_view = (tmp_output)[x];
		output_view.discard_data();

		parallel_for_each(fft_output_view.extent,
			[fft_output_view, output_view, scale](index<1> idx) restrict(amp)
		{
			auto real = fft_output_view[idx]._Val[_RE] * scale;
			auto img = fft_output_view[idx]._Val[_IM] * scale;
			auto amptitude = ampmath::sqrt(real * real + img * img) * 0.05f;

			auto dB = 10.0f * ampmath::log10(amptitude + EPSILON) / -60.0f;
			output_view[idx] = 1.0f - ampmath::fmin(1.0f, ampmath::fmax(0.0f, dB));
		});
	}
	std::vector<float> out = tmp_output;
	produce_spec_texture(tmp_output);
}

std::vector<uint32_t> ASIKCALL spectrogram_impl::get_output(size_t& width, size_t& height)
{
	width = output_buffer->extent[1];
	height = output_buffer->extent[0];

	std::vector<uint32_t> data;
	data.resize(output_buffer->data_length / 4);
	concurrency::graphics::copy(*output_buffer, data.data(), output_buffer->data_length);

	return std::move(data);
}

texture_view<uint_4, 2> spectrogram_impl::get_section() const
{
	return texture_view<uint_4, 2>(*output_buffer);
}

void spectrogram_impl::sample_fft_input(size_t step)
{
	auto begin = step * step_size;
	auto input_view = input_buffer->section(begin, fft_size);
	array_view<float, 1> fft_input_view(fft_input_buffer);
	fft_input_view.discard_data();

	auto N = fft_size;
	parallel_for_each(fft_input_view.extent, [=](index<1> idx)restrict(amp)
	{
		auto w = (1.0f - ampmath::cos((idx[0] * 2.0f * PI) / N)) * 0.5f;
		fft_input_view[idx] = input_view[idx] * w;
	});
}

uint abs(int value) restrict(amp)
{
	if (value < 0) return -value;
	return value;
}

void spectrogram_impl::produce_spec_texture(array_view<float, 2> tmp_output)
{
	int spec_height = height;
	graphics::texture_view<graphics::uint_4, 2> output_view(*output_buffer);
	parallel_for_each(tmp_output.extent, [tmp_output, output_view, spec_height](index<2> idx) restrict(amp)
	{
		uint_4 color;
		uint grey = tmp_output(idx) * 255;
		color = uint_4(grey, grey, grey, 255);
		output_view.set(index<2>(spec_height - idx[1] - 1, idx[0]), color);
	});
}

void ASIKCALL CreateSpectrogram(std::unique_ptr<NS_ASIK_CORE::spectrogram>& spec)
{
	spec = std::make_unique<spectrogram_impl>();
}