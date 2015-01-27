//
// Tomato ASIK
// MediaFoundtion IO 提供程序
// 内部使用
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#pragma once
#include "../../include/core/io/io.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

NSDEF_ASIK_CORE_IO

class mf_source_reader_handler;

// MediaFoundtion IO 提供程序
class mf_io_provider final : public io_provider
{
public:
	mf_io_provider();
	virtual ~mf_io_provider() noexcept;

	virtual void ASIKCALL set_output_type(const WAVEFORMATEX* Format, uint32_t FormatSize);
	virtual void ASIKCALL set_input(const string_t& FileName);
	virtual uint64_t ASIKCALL get_duration_hns() const noexcept;
	virtual concurrency::task<void> ASIKCALL read_sample(std::function<void(const byte* Buffer, size_t BufferSize)> handler);
private:
	static void initialize_mf();
	static void uninitialize_mf() noexcept;

	wrl::ComPtr<IMFAttributes> create_source_reader_attributes();
private:
	wrl::ComPtr<IMFSourceReader> sourceReader;
	wrl::ComPtr<mf_source_reader_handler> callbackHandler;
	wrl::ComPtr<IMFMediaType> outputMT;
	uint64_t duration;
};

NSED_ASIK_CORE_IO