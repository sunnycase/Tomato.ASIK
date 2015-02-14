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
	typedef std::pair<AVFrame, std::unique_ptr<byte[]>> convert_output_t;
public:
	ck_distance(size_t width, size_t height);
	~ck_distance() noexcept;

	float compute(const sample& sampleA, const sample& sampleB);
private:
	static void initialize_mf();
	static void uninitialize_mf() noexcept;
private:
	void create_mpeg_encoder();

	void create_g8toi420_converter();

	convert_output_t convert_rgb32_to_i420_sample(const sample& src);
	size_t get_mpeg_sample_length(convert_output_t& src1, convert_output_t& src2);
	size_t get_mpeg_sample_length(convert_output_t& src, int64_t pts, size_t& skipped);
	void reset_mpeg_encoder();
private:
	AVCodec* mpegEncoder;
	AVCodecContext* outputContext;
	SwsContext* g8toi420Context;
private:
	size_t width, height;
	size_t decode_height;
};

NSED_ASIK_CORE