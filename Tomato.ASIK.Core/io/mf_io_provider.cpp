//
// Tomato ASIK
// MediaFoundtion IO 提供程序
// 内部使用
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#include "stdafx.h"
#include "../stdafx.h"
#include "mf_io_provider.h"
#include "mf_source_reader_handler.h"

using namespace NS_ASIK_CORE_IO;
using namespace wrl;
using namespace concurrency;

mf_io_provider::mf_io_provider()
	:callbackHandler(Make<mf_source_reader_handler>())
{
	initialize_mf();
}

mf_io_provider::~mf_io_provider() noexcept
{
	uninitialize_mf();
}

void ASIKCALL mf_io_provider::set_output_type(const WAVEFORMATEX* Format, uint32_t FormatSize)
{
	ComPtr<IMFMediaType> partialMT;
	THROW_IF_FAILED(MFCreateMediaType(&partialMT));
	THROW_IF_FAILED(MFInitMediaTypeFromWaveFormatEx(partialMT.Get(), Format, FormatSize));

	// 只选择第一个音频流
	THROW_IF_FAILED(sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
	THROW_IF_FAILED(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true));

	// 配置SourceReader的媒体类型（自动加载解码器）
	THROW_IF_FAILED(sourceReader->SetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, partialMT.Get()));
	// 获取解码后的媒体类型
	THROW_IF_FAILED(sourceReader->GetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputMT));

	// 获取长度
	PROPVARIANT durationVar;
	if (SUCCEEDED(sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &durationVar)))
	{
		this->duration = durationVar.hVal.QuadPart;
		PropVariantClear(&durationVar);
	}
}

void ASIKCALL mf_io_provider::set_input(const string_t & FileName)
{
	auto attributes = create_source_reader_attributes();
	THROW_IF_FAILED(MFCreateSourceReaderFromURL(FileName.c_str(), attributes.Get(), &sourceReader));
}

uint64_t ASIKCALL mf_io_provider::get_duration_hns() const noexcept
{
	return duration;
}

task<void> ASIKCALL mf_io_provider::read_sample(std::function<void(const byte* Buffer, size_t BufferSize)> handler)
{
	task_completion_event<ComPtr<IMFMediaBuffer>> read_event;
	callbackHandler->set_current_read_event(read_event);

	THROW_IF_FAILED(sourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr,
		nullptr, nullptr, nullptr));
	return create_task(read_event).then([handler](ComPtr<IMFMediaBuffer> buffer)
	{
		BYTE* audioData = nullptr;
		DWORD audioDataLength = 0;

		// Lock the sample
		mfbuffer_locker locker(buffer.Get());
		THROW_IF_FAILED(locker.lock(&audioData, nullptr, &audioDataLength));
		handler(audioData, audioDataLength);
	});
}

void mf_io_provider::initialize_mf()
{
	THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
}

void mf_io_provider::uninitialize_mf() noexcept
{
	MFShutdown();
}

wrl::ComPtr<IMFAttributes> mf_io_provider::create_source_reader_attributes()
{
	ComPtr<IMFAttributes> attributes;

	THROW_IF_FAILED(MFCreateAttributes(&attributes, 3));
	// Specify Source Reader Attributes
	THROW_IF_FAILED(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callbackHandler.Get()));
	THROW_IF_FAILED(attributes->SetString(MF_READWRITE_MMCSS_CLASS_AUDIO, L"Audio"));
	THROW_IF_FAILED(attributes->SetUINT32(MF_READWRITE_MMCSS_PRIORITY_AUDIO, 0));

	return attributes;
}

ASIKAPI void ASIKCALL CreateMFIOProvider(std::unique_ptr<io_provider>& provider)
{
	provider = std::make_unique<mf_io_provider>();
}