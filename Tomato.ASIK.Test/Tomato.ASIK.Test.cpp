// Tomato.ASIK.Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace Tomato;
using namespace Tomato::ASIK::Core::IO;

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

int _tmain(int argc, _TCHAR* argv[])
{
	std::unique_ptr<io_provider> provider;
	CreateMFIOProvider(provider);

	provider->set_input(LR"(D:\Media\Music\Vocal\luv letter.mp3)");

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = 8000;
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
	provider->set_output_type(&format);

	block_buffer<byte> buffer(4096);
	read_sample(provider.get(), buffer).then([](concurrency::task<void> t)
	{
		try
		{
			t.get();
		}
		catch (...)
		{
			std::cout << "End of file." << std::endl;
		}
	}).get();

	system("pause");

	return 0;
}

