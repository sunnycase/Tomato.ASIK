//
// Tomato ASIK
// CK Distance
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-01
#include "stdafx.h"
#include "ck_distance.h"
#include "spectrogram_impl.h"
#include <fstream>

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;

ck_distance::ck_distance(size_t width, size_t height)
	:width(width), height(height), decode_height(height * 8)
{
	initialize_mf();
	create_g8toi420_converter();
	create_mpeg_encoder();
}

ck_distance::~ck_distance() noexcept
{
	uninitialize_mf();
}

float ck_distance::compute(const sample& sampleA, const sample& sampleB)
{
	auto xYUV = convert_rgb32_to_i420_sample(sampleA);
	auto yYUV = convert_rgb32_to_i420_sample(sampleB);

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

void ck_distance::create_g8toi420_converter()
{
	g8toi420Context = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_GRAY8,
		width, decode_height, AVPixelFormat::AV_PIX_FMT_YUV420P, SWS_BICUBIC,
		nullptr, nullptr, nullptr);
	THROW_IF_NOT(g8toi420Context, "Cannot Create Gray8 to I420 Converter.");
}

ck_distance::convert_output_t ck_distance::convert_rgb32_to_i420_sample(const sample& src)
{
	AVFrame input{ 0 }, output{ 0 };

	input.width = src.freq_extent;
	input.height = src.time_extent;
	avpicture_fill((AVPicture*)&input, (uint8_t*)src.data,
		AVPixelFormat::AV_PIX_FMT_GRAY8, input.width, input.height);

	auto output_buf = std::make_unique<uint8_t[]>(avpicture_get_size(
		AVPixelFormat::AV_PIX_FMT_YUV420P, width, decode_height));
	avpicture_fill((AVPicture*)&output, output_buf.get(),
		AVPixelFormat::AV_PIX_FMT_YUV420P, width, decode_height);

	sws_scale(g8toi420Context, input.data, input.linesize,
		0, input.height, output.data, output.linesize);

	return std::make_pair(std::move(output), std::move(output_buf));
}

size_t ck_distance::get_mpeg_sample_length(convert_output_t& src1, convert_output_t& src2)
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

size_t ck_distance::get_mpeg_sample_length(convert_output_t& src, int64_t pts, size_t & skipped)
{
	auto& inFrame = src.first;
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
	avcodec_free_context(&outputContext);
	create_mpeg_encoder();
}

void ck_distance::create_mpeg_encoder()
{
	mpegEncoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_MPEG1VIDEO);
	THROW_IF_NOT(mpegEncoder, "Cannot Find MPEG-2 Video Encoder.");

	outputContext = avcodec_alloc_context3(mpegEncoder);
	THROW_IF_NOT(outputContext, "Cannot Allocate Encoding Context.");
	outputContext->bit_rate = width * height * 4;
	outputContext->width = width;
	outputContext->height = height;
	outputContext->max_b_frames = 0;
	outputContext->gop_size = 2;
	outputContext->time_base = { 1, 25 };
	outputContext->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

	THROW_IF_NOT(avcodec_open2(outputContext, mpegEncoder, nullptr) >= 0,
		"Cannot Open MPEG-2 Video Encoder");
}