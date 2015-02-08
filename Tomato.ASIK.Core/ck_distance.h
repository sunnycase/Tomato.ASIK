//
// Tomato ASIK
// CK Distance
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-01
#pragma once
#include "../include/platform.h"
#include "../include/core/sample.h"

NSDEF_ASIK_CORE

class ck_distance
{
public:
	ck_distance(size_t width, size_t height);
	~ck_distance() noexcept;

	virtual float ASIKCALL compute(sample* sampleA, sample* sampleB);

	float compute(concurrency::array_view<uint32_t, 2> x, concurrency::array_view<uint32_t, 2> y);
private:
	static void initialize_mf();
	static void uninitialize_mf() noexcept;
private:
	void set_outputType();
	void set_h264InputType();
	void configure_encoder();
	void create_mf_h264_encoder_activator();
	void create_mf_h264_encoder();

	void create_mf_rgb32toIUV_converter();
	void configure_rgb32toIUV_converter();
	void create_mf_rgb32toIUV_activator();

	wrl::ComPtr<IMFSample> convert_rgb32_to_IYUV_sample(const concurrency::array_view<uint32_t, 2>& src);
	DWORD get_h264_sample_length(wrl::ComPtr<IMFSample> src1, wrl::ComPtr<IMFSample> src2);
	wrl::ComPtr<IMFSample> encode_h264_sample(wrl::ComPtr<IMFSample> src);
private:
	wrl::ComPtr<IMFActivate> h264EncoderActivator;
	wrl::ComPtr<IMFActivate> rgb32toIYUVActivator;
	wrl::ComPtr<IMFTransform> h264Encoder;
	wrl::ComPtr<ICodecAPI> h264Codec;
	wrl::ComPtr<IMFTransform> rgb32toIYUVConverter;
private:
	wrl::ComPtr<IMFMediaType> inputType;
	wrl::ComPtr<IMFMediaType> h264InputType;
	wrl::ComPtr<IMFMediaType> outputType;
	size_t width, height;
};

NSED_ASIK_CORE