//
// Tomato ASIK
// CK Distance
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-01
#include "stdafx.h"
#include "ck_distance.h"
#include "spectrogram_impl.h"
#include "sample_impl.h"

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;

ck_distance::ck_distance(size_t width, size_t height)
	:width(width), height(height)
{
	initialize_mf();
	create_mf_rgb32toi420_converter();
	create_mpeg_encoder();
}

ck_distance::~ck_distance() noexcept
{
	uninitialize_mf();
}

float ASIKCALL ck_distance::compute(sample* sampleA, sample* sampleB)
{
	auto x = (sample_impl*)sampleA;
	auto y = (sample_impl*)sampleB;

	return compute(x->view, y->view);
}

float ck_distance::compute(array_view<uint32_t, 2> x, array_view<uint32_t, 2> y)
{
	auto xYUV = convert_rgb32_to_i420_sample(x);
	auto yYUV = convert_rgb32_to_i420_sample(y);

	auto xy = get_mpeg_sample_length(xYUV, yYUV);
	auto yx = get_mpeg_sample_length(yYUV, xYUV);
	auto xx = get_mpeg_sample_length(xYUV, xYUV);
	auto yy = get_mpeg_sample_length(yYUV, yYUV);

	auto dist = (float)(xy + yx) / (float)(xx + yy) - 1.f;
	return std::abs(dist);
}

void ck_distance::initialize_mf()
{
	THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
	avcodec_register_all();
}

void ck_distance::uninitialize_mf() noexcept
{
	MFShutdown();
}

void ck_distance::create_mf_rgb32toi420_converter()
{
	MFT_REGISTER_TYPE_INFO inputType = { MFMediaType_Video, MFVideoFormat_RGB32 };
	MFT_REGISTER_TYPE_INFO outputType = { MFMediaType_Video, MFVideoFormat_I420 };

	ComPtr<IMFActivate>* activate;
	ComPtr<IMFActivate> rgb32toi420Activator;
	UINT32 activateNum = 0;
	THROW_IF_FAILED(MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, 0,
		&inputType, &outputType, reinterpret_cast<IMFActivate***>(&activate), &activateNum));
	if (activateNum)
	{
		rgb32toi420Activator = std::move(activate[0]);
		for (size_t i = 0; i < activateNum; i++)
			activate[i].Reset();
		CoTaskMemFree(activate);
	}
	else
		throw std::exception("Video Processor MFT Can't be created.");

	THROW_IF_FAILED(rgb32toi420Activator->ActivateObject(IID_PPV_ARGS(&rgb32toi420Converter)));
	configure_rgb32toi420_converter();
}

void ck_distance::configure_rgb32toi420_converter()
{
	HRESULT hr = S_OK;
	DWORD typeId = 0;
	GUID subType;
	while ((hr = rgb32toi420Converter->GetInputAvailableType(0,
		typeId++, rgbType.ReleaseAndGetAddressOf())) != MF_E_NO_MORE_TYPES)
	{
		THROW_IF_FAILED(hr);
		THROW_IF_FAILED(rgbType->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType == MFVideoFormat_RGB32)
			break;
	}
	if (subType != MFVideoFormat_RGB32)
		throw std::exception("Converter can't accept RGB32.");

	THROW_IF_FAILED(MFSetAttributeSize(rgbType.Get(), MF_MT_FRAME_SIZE, width, height));
	THROW_IF_FAILED(rgb32toi420Converter->SetInputType(0, rgbType.Get(), 0));

	typeId = 0;
	while ((hr = rgb32toi420Converter->GetOutputAvailableType(0,
		typeId++, yuvType.ReleaseAndGetAddressOf())) != MF_E_NO_MORE_TYPES)
	{
		THROW_IF_FAILED(hr);
		THROW_IF_FAILED(yuvType->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType == MFVideoFormat_I420)
			break;
	}
	if (subType != MFVideoFormat_I420)
		throw std::exception("Converter can't produce YUV420.");

	THROW_IF_FAILED(MFSetAttributeSize(yuvType.Get(), MF_MT_FRAME_SIZE, width, height));
	THROW_IF_FAILED(rgb32toi420Converter->SetOutputType(0, yuvType.Get(), 0));
}

ComPtr<IMFSample> ck_distance::convert_rgb32_to_i420_sample(const array_view<uint32_t, 2>& src)
{
	THROW_IF_FAILED(rgb32toi420Converter->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));
	ComPtr<IMFMediaBuffer> inputBuffer;
	THROW_IF_FAILED(MFCreate2DMediaBuffer(src.extent[1], src.extent[0], MFVideoFormat_RGB32.Data1,
		FALSE, &inputBuffer));

	auto count = src.extent.size();
	{
		BYTE* data; DWORD maxLen, curLen;
		mfbuffer_locker locker(inputBuffer.Get());
		THROW_IF_FAILED(locker.lock(&data, &maxLen, &curLen));
		concurrency::copy(src, stdext::make_unchecked_array_iterator((uint32_t*)data));
	}
	THROW_IF_FAILED(inputBuffer->SetCurrentLength(count * sizeof(uint32_t)));

	ComPtr<IMFSample> inputSample;
	THROW_IF_FAILED(MFCreateSample(&inputSample));
	THROW_IF_FAILED(inputSample->AddBuffer(inputBuffer.Get()));
	THROW_IF_FAILED(inputSample->SetSampleTime(0));
	THROW_IF_FAILED(inputSample->SetSampleDuration(1 * 1e7));
	THROW_IF_FAILED(rgb32toi420Converter->ProcessInput(0, inputSample.Get(), 0));

	MFT_OUTPUT_STREAM_INFO outputInfo;
	THROW_IF_FAILED(rgb32toi420Converter->GetOutputStreamInfo(0, &outputInfo));
	ComPtr<IMFMediaBuffer> outputBuffer;
	THROW_IF_FAILED(MFCreateMediaBufferFromMediaType(yuvType.Get(), 0,
		outputInfo.cbSize, outputInfo.cbAlignment, &outputBuffer));
	DWORD outputStatus = 0;
	MFT_OUTPUT_DATA_BUFFER outputData = { 0 };
	ComPtr<IMFSample> outputSample;
	THROW_IF_FAILED(MFCreateSample(&outputSample));
	THROW_IF_FAILED(outputSample->AddBuffer(outputBuffer.Get()));
	outputData.pSample = outputSample.Get();
	THROW_IF_FAILED(rgb32toi420Converter->ProcessOutput(0, 1, &outputData, &outputStatus));

	return outputSample;
}

size_t ck_distance::get_mpeg_sample_length(wrl::ComPtr<IMFSample> src1, wrl::ComPtr<IMFSample> src2)
{
	size_t totalSize = 0;
	size_t skipFrame = 0;

	totalSize += get_mpeg_sample_length(src1, 0, skipFrame);
	totalSize += get_mpeg_sample_length(src2, 1, skipFrame);

	while (skipFrame)
	{
		AVPacket packet;
		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;

		int got_packet = 0;
		THROW_IF_NOT(avcodec_encode_video2(outputContext, &packet, nullptr, &got_packet) == 0,
			"Encoding Error.");
		if (got_packet)
		{
			totalSize += packet.size;
			av_free_packet(&packet);
			skipFrame--;
		}
	}
	reset_mpeg_encoder();

	return totalSize;
}

size_t ck_distance::get_mpeg_sample_length(wrl::ComPtr<IMFSample> src, int64_t pts, size_t & skipped)
{
	AVFrame inFrame{ 0 };
	ComPtr<IMFMediaBuffer> inBuffer;
	THROW_IF_FAILED(src->GetBufferByIndex(0, inBuffer.ReleaseAndGetAddressOf()));
	BYTE* indata; DWORD maxLen, curLen;
	mfbuffer_locker locker(inBuffer.Get());
	THROW_IF_FAILED(locker.lock(&indata, &maxLen, &curLen));
	inFrame.width = width;
	inFrame.height = height;
	inFrame.format = AV_PIX_FMT_YUV420P;
	avpicture_fill((AVPicture*)&inFrame, indata, AVPixelFormat::AV_PIX_FMT_YUV420P, width, height);
	inFrame.pts = pts;

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;

	int got_packet = 0;
	THROW_IF_NOT(avcodec_encode_video2(outputContext, &packet, &inFrame, &got_packet) == 0,
		"Encoding Error.");
	if (got_packet)
	{
		auto size = packet.size;
		av_free_packet(&packet);
		return size;
	}
	else
	{
		skipped++;
		return 0;
	}
}

void ck_distance::reset_mpeg_encoder()
{
	THROW_IF_NOT(avcodec_close(outputContext) == 0,
		"Cannot Close MPEG-2 Video Encoder Context.");
	THROW_IF_NOT(avcodec_open2(outputContext, mpegEncoder, nullptr) >= 0,
		"Cannot Open MPEG-2 Video Encoder Context.");
}

void ck_distance::create_mpeg_encoder()
{
	mpegEncoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_MPEG2VIDEO);
	THROW_IF_NOT(mpegEncoder, "Cannot Find MPEG-2 Video Encoder.");

	outputContext = avcodec_alloc_context3(mpegEncoder);
	THROW_IF_NOT(outputContext, "Cannot Allocate Encoding Context.");
	outputContext->bit_rate = width * height * 400;
	outputContext->width = width;
	outputContext->height = height;
	outputContext->max_b_frames = 0;
	outputContext->gop_size = 2;
	outputContext->time_base = { 1, 25 };
	outputContext->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

	THROW_IF_NOT(avcodec_open2(outputContext, mpegEncoder, nullptr) >= 0,
		"Cannot Open MPEG-2 Video Encoder");
}