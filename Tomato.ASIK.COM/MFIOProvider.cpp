// MFIOProvider.cpp : CMFIOProvider µÄÊµÏÖ

#include "stdafx.h"
#include "MFIOProvider.h"

using namespace Tomato;
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
	format.nSamplesPerSec = 8000;
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
	*bufferSize = buffer.tell_not_get();
	return S_OK;
}


STDMETHODIMP CMFIOProvider::ReadAllSamples(SAFEARRAY* buffer)
{
	this->buffer.read((BYTE*)buffer->pvData, buffer->rgsabound[0].cElements);
	return S_OK;
}
