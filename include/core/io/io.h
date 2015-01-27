//
// Tomato ASIK
// I/O 相关
//
// (c) 2014 SunnyCase
// 创建日期: 2014-12-19
#pragma once
#include "../../platform.h"
#include <mmreg.h>

#define NSDEF_ASIK_CORE_IO namespace Tomato{namespace ASIK{namespace Core{namespace IO{
#define NSED_ASIK_CORE_IO }}}}
#define NS_ASIK_CORE_IO Tomato::ASIK::Core::IO

NSDEF_ASIK_CORE_IO

// IO 提供程序
class io_provider
{
public:
	virtual void ASIKCALL set_output_type(const WAVEFORMATEX* Format, uint32_t FormatSize = sizeof(WAVEFORMATEX)) = 0;
	virtual void ASIKCALL set_input(const string_t& FileName) = 0;
	virtual uint64_t ASIKCALL get_duration_hns() const noexcept = 0;
	virtual concurrency::task<void> ASIKCALL read_sample(std::function<void(const byte* Buffer, size_t BufferSize)> handler) = 0;
};

NSED_ASIK_CORE_IO

extern "C" ASIKAPI void ASIKCALL CreateMFIOProvider(std::unique_ptr<NS_ASIK_CORE_IO::io_provider>& provider);