//
// Tomato ASIK
// CK Distance
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-01
#include "stdafx.h"
#include "ck_distance_impl.h"
#include "spectrogram_impl.h"

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency::graphics;

ck_distance_impl::ck_distance_impl(size_t width, size_t height)
	:width(width), height(height)
{
	initialize_mf();
	create_mf_h264_encoder_activator();
	create_mf_h264_encoder();
	create_mf_rgb32toIUV_activator();
	create_mf_rgb32toIUV_converter();
}

ck_distance_impl::~ck_distance_impl() noexcept
{
	uninitialize_mf();
}

float ck_distance_impl::compute(spectrogram * specA, spectrogram * specB)
{
	auto specA_impl = (spectrogram_impl*)specA;
	auto specB_impl = (spectrogram_impl*)specB;
	auto x = specA_impl->get_section();
	auto y = specB_impl->get_section();

	return compute(x, y);
}

float ck_distance_impl::compute(texture_view<uint_4, 2> x, texture_view<uint_4, 2> y)
{
	auto xYUV = convert_rgb32_to_IYUV_sample(x);
	auto yYUV = convert_rgb32_to_IYUV_sample(y);

	auto xy = get_h264_sample_length(xYUV, yYUV);
	auto yx = get_h264_sample_length(yYUV, xYUV);
	auto xx = get_h264_sample_length(xYUV, xYUV);
	auto yy = get_h264_sample_length(yYUV, yYUV);

	return (float)(xy + yx) / (float)(xx + yy) - 1.f;
}

void ck_distance_impl::initialize_mf()
{
	THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
}

void ck_distance_impl::uninitialize_mf() noexcept
{
	MFShutdown();
}

void ck_distance_impl::set_outputType()
{
	HRESULT hr = S_OK;
	DWORD typeId = 0;
	GUID subType;
	while ((hr = h264Encoder->GetOutputAvailableType(0,
		typeId++, outputType.ReleaseAndGetAddressOf())) != MF_E_NO_MORE_TYPES)
	{
		THROW_IF_FAILED(hr);
		THROW_IF_FAILED(outputType->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType == MFVideoFormat_H264)
			break;
	}
	if (subType != MFVideoFormat_H264)
		throw std::exception("Encoder can't produce H.264.");

	THROW_IF_FAILED(outputType->SetUINT32(MF_MT_AVG_BITRATE, width * height * 4));
	THROW_IF_FAILED(MFSetAttributeSize(outputType.Get(), MF_MT_FRAME_SIZE, width, height));
	THROW_IF_FAILED(MFSetAttributeRatio(outputType.Get(), MF_MT_FRAME_RATE, 2, 1));
	THROW_IF_FAILED(outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
	THROW_IF_FAILED(h264Encoder->SetOutputType(0, outputType.Get(), 0));
}

void ck_distance_impl::set_h264InputType()
{
	HRESULT hr = S_OK;
	DWORD typeId = 0;
	GUID subType;
	while ((hr = h264Encoder->GetInputAvailableType(0,
		typeId++, h264InputType.ReleaseAndGetAddressOf())) != MF_E_NO_MORE_TYPES)
	{
		THROW_IF_FAILED(hr);
		THROW_IF_FAILED(h264InputType->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType == MFVideoFormat_IYUV)
			break;
	}
	if (subType != MFVideoFormat_IYUV)
		throw std::exception("Encoder can't accept IYUV.");

	THROW_IF_FAILED(MFSetAttributeSize(h264InputType.Get(), MF_MT_FRAME_SIZE, width, height));
	THROW_IF_FAILED(MFSetAttributeRatio(h264InputType.Get(), MF_MT_FRAME_RATE, 2, 1));
	THROW_IF_FAILED(h264Encoder->SetInputType(0, h264InputType.Get(), 0));
}

void ck_distance_impl::configure_encoder()
{
	THROW_IF_FAILED(h264Codec->SetValue(&CODECAPI_AVLowLatencyMode, &ATL::CComVariant(true)));

	set_outputType();
	set_h264InputType();
}

void ck_distance_impl::create_mf_rgb32toIUV_converter()
{
	THROW_IF_FAILED(rgb32toIYUVActivator->ActivateObject(IID_PPV_ARGS(&rgb32toIYUVConverter)));
	configure_rgb32toIUV_converter();
}

void ck_distance_impl::configure_rgb32toIUV_converter()
{
	THROW_IF_FAILED(rgb32toIYUVConverter->SetOutputType(0, h264InputType.Get(), 0));

	HRESULT hr = S_OK;
	DWORD typeId = 0;
	GUID subType;
	while ((hr = rgb32toIYUVConverter->GetInputAvailableType(0,
		typeId++, inputType.ReleaseAndGetAddressOf())) != MF_E_NO_MORE_TYPES)
	{
		THROW_IF_FAILED(hr);
		THROW_IF_FAILED(inputType->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType == MFVideoFormat_RGB32)
			break;
	}
	if (subType != MFVideoFormat_RGB32)
		throw std::exception("Converter can't accept RGB32.");

	THROW_IF_FAILED(MFSetAttributeSize(inputType.Get(), MF_MT_FRAME_SIZE, width, height));
	THROW_IF_FAILED(rgb32toIYUVConverter->SetInputType(0, inputType.Get(), 0));
}

void ck_distance_impl::create_mf_rgb32toIUV_activator()
{
	MFT_REGISTER_TYPE_INFO inputType = { MFMediaType_Video, MFVideoFormat_RGB32 };
	MFT_REGISTER_TYPE_INFO outputType = { MFMediaType_Video, MFVideoFormat_IYUV };

	ComPtr<IMFActivate>* activate;
	UINT32 activateNum = 0;
	THROW_IF_FAILED(MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, 0,
		&inputType, &outputType, reinterpret_cast<IMFActivate***>(&activate), &activateNum));
	if (activateNum)
	{
		rgb32toIYUVActivator = activate[0];
		for (size_t i = 0; i < activateNum; i++)
			activate[i].Reset();
		CoTaskMemFree(activate);
	}
	else
		throw std::exception("Video Processor MFT Can't be created.");
}

ComPtr<IMFSample> ck_distance_impl::convert_rgb32_to_IYUV_sample(texture_view<uint_4, 2> src)
{
	THROW_IF_FAILED(rgb32toIYUVConverter->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
	ComPtr<IMFMediaBuffer> inputBuffer;
	THROW_IF_FAILED(MFCreate2DMediaBuffer(src.extent[1], src.extent[0], MFVideoFormat_RGB32.Data1,
		FALSE, &inputBuffer));

	{
		BYTE* data; DWORD maxLen, curLen;
		mfbuffer_locker locker(inputBuffer.Get());
		THROW_IF_FAILED(locker.lock(&data, &maxLen, &curLen));
		concurrency::graphics::copy(src, data, src.data_length);
	}
	THROW_IF_FAILED(inputBuffer->SetCurrentLength(src.data_length));

	ComPtr<IMFSample> inputSample;
	THROW_IF_FAILED(MFCreateSample(&inputSample));
	THROW_IF_FAILED(inputSample->AddBuffer(inputBuffer.Get()));
	THROW_IF_FAILED(inputSample->SetSampleTime(0));
	THROW_IF_FAILED(inputSample->SetSampleDuration(1 * 1e7));
	THROW_IF_FAILED(rgb32toIYUVConverter->ProcessInput(0, inputSample.Get(), 0));

	MFT_OUTPUT_STREAM_INFO outputInfo;
	THROW_IF_FAILED(rgb32toIYUVConverter->GetOutputStreamInfo(0, &outputInfo));
	ComPtr<IMFMediaBuffer> outputBuffer;
	THROW_IF_FAILED(MFCreateMediaBufferFromMediaType(h264InputType.Get(), 0,
		outputInfo.cbSize, outputInfo.cbAlignment, &outputBuffer));
	DWORD outputStatus = 0;
	MFT_OUTPUT_DATA_BUFFER outputData = { 0 };
	ComPtr<IMFSample> outputSample;
	THROW_IF_FAILED(MFCreateSample(&outputSample));
	THROW_IF_FAILED(outputSample->AddBuffer(outputBuffer.Get()));
	outputData.pSample = outputSample.Get();
	THROW_IF_FAILED(rgb32toIYUVConverter->ProcessOutput(0, 1, &outputData, &outputStatus));
	THROW_IF_FAILED(rgb32toIYUVConverter->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));

	return outputSample;
}

DWORD ck_distance_impl::get_h264_sample_length(wrl::ComPtr<IMFSample> src1, wrl::ComPtr<IMFSample> src2)
{
	THROW_IF_FAILED(h264Encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
	auto encoded1 = encode_h264_sample(src1);
	auto encoded2 = encode_h264_sample(src2);

	THROW_IF_FAILED(h264Encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0));
	THROW_IF_FAILED(h264Encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0));
	THROW_IF_FAILED(h264Encoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));

	DWORD size1, size2;
	THROW_IF_FAILED(encoded1->GetTotalLength(&size1));
	THROW_IF_FAILED(encoded2->GetTotalLength(&size2));
	create_mf_h264_encoder();
	return size1 + size2;
}

wrl::ComPtr<IMFSample> ck_distance_impl::encode_h264_sample(wrl::ComPtr<IMFSample> src)
{
	THROW_IF_FAILED(h264Encoder->ProcessInput(0, src.Get(), 0));

	MFT_OUTPUT_STREAM_INFO outputInfo;
	THROW_IF_FAILED(h264Encoder->GetOutputStreamInfo(0, &outputInfo));
	ComPtr<IMFMediaBuffer> outputBuffer;
	THROW_IF_FAILED(MFCreateMediaBufferFromMediaType(outputType.Get(), 0,
		outputInfo.cbSize, outputInfo.cbAlignment, &outputBuffer));
	DWORD outputStatus = 0;
	MFT_OUTPUT_DATA_BUFFER outputData = { 0 };
	ComPtr<IMFSample> outputSample;
	THROW_IF_FAILED(MFCreateSample(&outputSample));
	THROW_IF_FAILED(outputSample->AddBuffer(outputBuffer.Get()));
	outputData.pSample = outputSample.Get();
	THROW_IF_FAILED(h264Encoder->ProcessOutput(0, 1, &outputData, &outputStatus));
	return outputSample;
}

void ck_distance_impl::create_mf_h264_encoder_activator()
{
	MFT_REGISTER_TYPE_INFO outputType = { MFMediaType_Video, MFVideoFormat_H264 };

	ComPtr<IMFActivate>* activate;
	UINT32 activateNum = 0;
	THROW_IF_FAILED(MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, 0,
		nullptr, &outputType, reinterpret_cast<IMFActivate***>(&activate), &activateNum));
	if (activateNum)
	{
		h264EncoderActivator = activate[0];
		for (size_t i = 0; i < activateNum; i++)
			activate[i].Reset();
		CoTaskMemFree(activate);
	}
	else
		throw std::exception("H.264 Encoder Can't be created.");
}

void ck_distance_impl::create_mf_h264_encoder()
{
	THROW_IF_FAILED(h264EncoderActivator->ActivateObject(IID_PPV_ARGS(h264Encoder.ReleaseAndGetAddressOf())));
	h264Codec.Reset();
	THROW_IF_FAILED(h264Encoder.As(&h264Codec));
	configure_encoder();
}

void ASIKCALL CreateCKDistance(std::unique_ptr<NS_ASIK_CORE::ck_distance>& dist, size_t width, size_t height)
{
	dist = std::make_unique<ck_distance_impl>(width, height);
}