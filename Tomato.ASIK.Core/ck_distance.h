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
	void create_mpeg_encoder();

	void create_mf_rgb32toi420_converter();
	void configure_rgb32toi420_converter();

	wrl::ComPtr<IMFSample> convert_rgb32_to_i420_sample(const concurrency::array_view<uint32_t, 2>& src);
	size_t get_mpeg_sample_length(wrl::ComPtr<IMFSample> src1, wrl::ComPtr<IMFSample> src2);
	size_t get_mpeg_sample_length(wrl::ComPtr<IMFSample> src, int64_t pts, size_t& skipped);
	void reset_mpeg_encoder();
private:
	wrl::ComPtr<IMFTransform> rgb32toi420Converter;
	AVCodec* mpegEncoder;
	AVCodecContext* outputContext;
private:
	wrl::ComPtr<IMFMediaType> rgbType;
	wrl::ComPtr<IMFMediaType> yuvType;
	size_t width, height;
};

NSED_ASIK_CORE