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
	auto align_len = input_len - input_len % fft_size;
	if (align_len < input_len) align_len += fft_size;
	auto data = std::make_unique<float[]>(align_len);
	ZeroMemory(data.get(), align_len * sizeof(float));
	auto end = data.get() + input_len;
	for (auto p = data.get(); p != end; ++p)
		*p = *input++ / 32767.9f;
	input_buffer = std::make_unique<array<float, 1>>(extent<1>(align_len), data.get());

	_time_extent = (align_len - fft_size) / step_size;
	freq_extent = fft_size / 2;
	output_buffer = std::make_unique<byte[]>(_time_extent * freq_extent);
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
	array<float, 2> tmp_output((int)_time_extent, (int)freq_extent);

	for (size_t x = 0; x < time_extent; x++)
	{
		sample_fft_input(x);
		fft_transform->forward_transform(fft_input_buffer, fft_output_buffer);
		array_view<std::complex<float>, 1> fft_output_view(fft_output_buffer);
		auto output_view = (tmp_output)[x];
		output_view.discard_data();

		parallel_for_each(fft_output_view.extent,
			[fft_output_view, output_view, scale](index<1> idx) restrict(amp)
		{
			auto real = fft_output_view[idx]._Val[_RE];
			auto img = fft_output_view[idx]._Val[_IM];
			auto amptitude = ampmath::sqrt(real * real + img * img) * scale;

			auto dB = amptitude == 0.f ? 1.f : 10.0f * ampmath::log10(amptitude) / -96.f;
			output_view[idx] = 1.0f - ampmath::fmin(1.0f, ampmath::fmax(0.0f, dB));
		});
	}
	produce_spec_texture(tmp_output);
}

size_t ASIKCALL spectrogram_impl::get_time_extent()
{
	ensure_spectorgram_created();
	return _time_extent;
}

sample ASIKCALL spectrogram_impl::get_sample(size_t start_time, size_t time_extent)
{
	if (start_time + time_extent > this->_time_extent)
		throw std::out_of_range("Start time or Time extent is out of range.");

	ensure_spectorgram_created();
	return sample(output_buffer.get(), freq_extent, time_extent, start_time);
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

void spectrogram_impl::produce_spec_texture(const array_view<float, 2>& tmp_output)
{
	array<uint32_t, 2> output((int)_time_extent, (int)freq_extent);
	int spec_height = freq_extent;
	parallel_for_each(tmp_output.extent, [tmp_output, &output, spec_height](index<2> idx) restrict(amp)
	{
		uint32_t grey = tmp_output(idx) * 255;
		output(idx[0], idx[1]) = grey;
	});
	std::vector<uint32_t> data = output;
	auto beg = output_buffer.get();
	for (auto value : data)
		*beg++ = (byte)value;
}

void spectrogram_impl::ensure_spectorgram_created()
{
	if (!output_buffer)
		draw();
}

void ASIKCALL CreateSpectrogram(std::unique_ptr<NS_ASIK_CORE::spectrogram>& spec)
{
	spec = std::make_unique<spectrogram_impl>();
}