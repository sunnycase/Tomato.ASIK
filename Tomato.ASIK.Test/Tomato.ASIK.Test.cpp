// Tomato.ASIK.Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace Tomato;
using namespace Tomato::ASIK::Core;
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

std::vector<uint32_t> produce_spectrogram(const WAVEFORMATEX* format, const std::wstring& fileName)
{
	std::unique_ptr<io_provider> provider;
	CreateMFIOProvider(provider);

	provider->set_input(fileName);
	provider->set_output_type(format);

	auto time1 = clock();
	block_buffer<byte> buffer(4096);
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
	auto samples_count = buffer.tell_not_get() / sizeof(short);
	auto samples = std::make_unique<short[]>(samples_count);
	buffer.read((byte*)samples.get(), buffer.tell_not_get());

	std::unique_ptr<spectrogram> spectrogram;
	CreateSpectrogram(spectrogram);
	spectrogram->set_input(samples.get(), samples_count);
	spectrogram->draw();
	size_t width, height;
	auto image_data = spectrogram->get_output(width, height);
	auto time2 = clock();

	auto relFileName = fileName.substr(fileName.find_last_of(L'\\') + 1);
	std::wcout << relFileName << L'(' << width << L'x' << height << L") Used: "
		<< float(time2 - time1) / CLOCKS_PER_SEC << L"sec." << std::endl;
	return image_data;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale(""));

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = 44100;
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

	for (size_t i = 1; i <= 500; i++)
	{
		std::wstringstream ss;
		ss << LR"(D:\Work\Projects\Science\Tomato.ASIK\references\UCR_Contest\Train\)";
		ss << std::setfill(L'0') << std::setw(4) << i << L".wav";

		produce_spectrogram(&format, ss.str());
	}

	system("pause");

	return 0;
}

