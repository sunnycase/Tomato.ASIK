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
#include "ck_distance.h"

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

	int spec_height = height;
	graphics::texture_view<graphics::uint_4, 2> output_view(*output_buffer);
	parallel_for_each(tmp_output.extent, [&tmp_output, output_view, spec_height](index<2> idx) restrict(amp)
	{
		uint32_t r = tmp_output(idx) * 255;
		output_view.set(index<2>(spec_height - idx[1] - 1, idx[0]), graphics::uint_4(r, r, r, 255));
	});

	ck_distance ck(output_view.extent[1], output_view.extent[0]);
	auto inputSample1 = ck.convert_rgb32_to_IYUV_sample(output_view);
	auto inputSample2 = ck.convert_rgb32_to_IYUV_sample(output_view);
	ck.encode_h264_sample(inputSample1, inputSample2);
}

std::vector<uint32_t> ASIKCALL spectrogram_impl::get_output(size_t& width, size_t& height)
{
	width = this->width;
	height = this->height;

	std::vector<uint32_t> data;
	data.resize(output_buffer->data_length / 4);
	concurrency::graphics::copy(*output_buffer, data.data(), output_buffer->data_length);

	return std::move(data);
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