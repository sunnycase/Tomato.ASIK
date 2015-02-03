// MFIOProvider.cpp : CMFIOProvider µÄÊµÏÖ

#include "stdafx.h"
#include "MFIOProvider.h"

using namespace Tomato;
using namespace Tomato::ASIK::Core;
using namespace Tomato::ASIK::Core::IO;

// CMFIOProvider

concurrency::task<void> read_sample(io_provider* provider, block_buffer<byte>& buffer)
{
	return provider->read_sample([&buffer](const byte* Buffer, size_t BufferSize)
	{
		buffer.write(Buffer, BufferSize);
	}).then([provider, &buffer]
	{
		return read_sample(provider, buffer);
	});
}


STDMETHODIMP CMFIOProvider::Initialize()
{
	CreateMFIOProvider(provider);
	return S_OK;
}


STDMETHODIMP CMFIOProvider::LoadFile(LPCWSTR fileName, DWORD* bufferSize)
{

	provider->set_input(fileName);

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = 16000;
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
	provider->set_output_type(&format);

	read_sample(provider.get(), buffer).then([](concurrency::task<void> t)
	{
		try
		{
			t.get();
		}
		catch (...)
		{
		}
	}).get();

	samples_count = this->buffer.tell_not_get() / sizeof(short);
	samples = std::make_unique<short[]>(samples_count);
	this->buffer.read((byte*)samples.get(), this->buffer.tell_not_get());

	CreateSpectrogram(spectr);
	spectr->set_input(samples.get(), samples_count);
	*bufferSize = samples_count * sizeof(short);
	return S_OK;
}


STDMETHODIMP CMFIOProvider::ReadAllSamples(SAFEARRAY* buffer)
{
	memcpy_s((BYTE*)buffer->pvData, buffer->rgsabound[0].cElements, samples.get(), samples_count * sizeof(short));

	return S_OK;
}


STDMETHODIMP CMFIOProvider::PrepareSpectrogram(DWORD* width, DWORD* height)
{
	if (specData.empty())
	{
		spectr->draw();
		specData = spectr->get_output(img_Width, img_Height);
	}
	*width = img_Width;
	*height = img_Height;
	return S_OK;
}

STDMETHODIMP CMFIOProvider::DrawSpectrogram(SAFEARRAY * buffer)
{
	memcpy_s((FLOAT*)buffer->pvData, buffer->rgsabound[0].cElements * sizeof(uint32_t),
		specData.data(), specData.size() * sizeof(uint32_t));
	return S_OK;
}
