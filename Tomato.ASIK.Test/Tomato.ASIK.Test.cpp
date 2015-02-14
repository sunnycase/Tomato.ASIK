// Tomato.ASIK.Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace Tomato;
using namespace Tomato::ASIK::Core;
using namespace Tomato::ASIK::Core::IO;
using namespace Tomato::ASIK::Core::Classifier;

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

std::unique_ptr<spectrogram> produce_spectrogram(const WAVEFORMATEX* format, const std::wstring& fileName)
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

	auto sample = spectrogram->get_sample();
	auto image_data = sample.data;

	auto time2 = clock();

	auto relFileName = fileName.substr(fileName.find_last_of(L'\\') + 1);
	std::wcout << relFileName << L'(' << sample.freq_extent << L'x' << sample.time_extent << L") Used: "
		<< float(time2 - time1) / CLOCKS_PER_SEC << L"sec." << std::endl;
	return std::move(spectrogram);
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale(""));

	WAVEFORMATEX format = { 0 };
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = 16000;
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

	std::vector<std::unique_ptr<spectrogram>> specs;
	for (size_t i = 1; i <= 20; i++)
	{
		std::wstringstream ss;
		ss << LR"(D:\Work\Projects\Science\Tomato.ASIK\references\UCR_Contest\Train\)";
		ss << std::setfill(L'0') << std::setw(4) << i << L".wav";

		specs.emplace_back(produce_spectrogram(&format, ss.str()));
	}

	std::unique_ptr<ck_distance_service> ck;
	CreateCKDistanceService(256, ck);
#if 0
	std::unique_ptr<classifier> clsifier;
	CreateClassifier(1, 20, clsifier);
	class_id_t ids[] = { 3, 4, 4, 5, 1, 5, 3, 1, 5, 2,
	5, 2, 3, 2, 4, 4, 2, 4, 2, 3};
	for (size_t i = 0; i < ARRAYSIZE(ids); i++)
	{
		clsifier->add_input(ids[i], std::move(specs[i]));
	}
	clsifier->set_ck_distance_service(ck.get());
	clsifier->compute_fingerprint(3);
#else

	for (size_t x = 0; x < 10; x++)
	{
		for (size_t y = 0; y < 10; y++)
		{
			std::wcout << std::setfill(L'0') << std::setw(4) << x + 1 << L".wav VS " <<
				std::setfill(L'0') << std::setw(4) << y + 1 << L".wav CK Distance: ";
			auto sampleX = specs[x]->get_sample(0, 62);
			auto sampleY = specs[y]->get_sample(0, 62);
			auto dist = ck->compute(sampleX, sampleY);
			std::wcout << dist << std::endl;
		}
	}
#endif

	system("pause");
	return 0;
}

